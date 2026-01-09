"""
XML 기반 Sample JSON 생성 도구

이 스크립트는 DDS XML 정의 파일들을 파싱하여, 'C_'로 시작하는 구조체(Struct)에 대한
Sample JSON 파일들을 생성합니다.

사용법:
    python generate_sample_json.py <입력_XML_파일_1> [<입력_XML_파일_2> ...] [-o 출력_디렉토리]

인자:
    <입력_XML_파일_n>: 처리할 DDS XML 파일 경로. 하나 이상의 파일을 지정할 수 있으며,
                       와일드카드(*.xml) 패턴도 지원합니다.
    -o, --output     : (선택) 생성된 JSON 파일들이 저장될 폴더 경로. 기본값은 'output_json' 입니다.

예시:
    python Tools/XmlToSampleJson/generate_sample_json.py config/generated/*.xml -o output_json

참고:
    생성되는 JSON 파일 이름은 '모듈명::구조체명'을 기준으로 하며, 파일 시스템 제약으로 ':' 등
    특수 문자는 '_'로 대체됩니다.
"""
import os
import sys
import json
import xml.etree.ElementTree as ET
import glob
from typing import Dict, Any, List, Optional


INVALID_FILENAME_CHARS = '<>:"/\\|?*'


def sanitize_filename(name: str) -> str:
    """파일 시스템에서 사용할 수 있도록 이름을 정제한다."""
    sanitized = name
    for ch in INVALID_FILENAME_CHARS:
        sanitized = sanitized.replace(ch, '_')
    return sanitized.replace(' ', '_')


def iter_structs_with_module(element: ET.Element, current_module: Optional[str] = None):
    """현재 XML 노드 기준으로 모듈-구조체 쌍을 순회한다."""
    module_name = current_module
    if element.tag == 'module':
        module_name = element.get('name') or current_module

    if element.tag == 'struct':
        yield module_name, element

    for child in element:
        yield from iter_structs_with_module(child, module_name)


class TypeRegistry:
    def __init__(self):
        self.typedefs: Dict[str, str] = {}  # name -> type (or nonBasicTypeName)
        self.enums: Dict[str, List[str]] = {}  # name -> list of enumerator names
        self.structs: Dict[str, ET.Element] = {}  # name -> struct Element
        self.short_name_map: Dict[str, str] = {}  # short_name -> full_name

    def register_typedef(self, name: str, type_name: str):
        self.typedefs[name] = type_name
        self._register_short_name(name)

    def register_enum(self, name: str, values: List[str]):
        self.enums[name] = values
        self._register_short_name(name)

    def register_struct(self, name: str, element: ET.Element):
        self.structs[name] = element
        self._register_short_name(name)

    def _register_short_name(self, full_name: str):
        if '::' in full_name:
            short_name = full_name.split('::')[-1]
            self.short_name_map[short_name] = full_name
        else:
            self.short_name_map[full_name] = full_name

    def resolve_type(self, type_name: str) -> str:
        """Recursively resolves typedefs to the underlying type."""
        # Try exact match
        if type_name in self.typedefs:
            return self.resolve_type(self.typedefs[type_name])
        
        # Try short name match
        short_name = type_name.split('::')[-1]
        if short_name in self.short_name_map:
            full_name = self.short_name_map[short_name]
            if full_name in self.typedefs:
                return self.resolve_type(self.typedefs[full_name])
            return full_name # It might be a struct or enum found by short name

        return type_name

    def get_enum_values(self, type_name: str) -> Optional[List[str]]:
        resolved = self.resolve_type(type_name)
        if resolved in self.enums:
            return self.enums[resolved]
        
        # Try short name lookup for enums
        short_name = resolved.split('::')[-1]
        if short_name in self.short_name_map:
            full_name = self.short_name_map[short_name]
            if full_name in self.enums:
                return self.enums[full_name]
        return None

    def get_struct(self, type_name: str) -> Optional[ET.Element]:
        resolved = self.resolve_type(type_name)
        if resolved in self.structs:
            return self.structs[resolved]
        
        # Try short name lookup for structs
        short_name = resolved.split('::')[-1]
        if short_name in self.short_name_map:
            full_name = self.short_name_map[short_name]
            if full_name in self.structs:
                return self.structs[full_name]
        return None

class SchemaParser:
    def __init__(self, registry: TypeRegistry):
        self.registry = registry

    def parse_directory(self, directory: str):
        """Parses all XML files in the directory to populate the registry."""
        xml_files = glob.glob(os.path.join(directory, "*.xml"))
        for xml_file in xml_files:
            try:
                tree = ET.parse(xml_file)
                root = tree.getroot()
                self._parse_types(root)
            except ET.ParseError as e:
                print(f"Warning: Failed to parse {xml_file}: {e}")

    def _parse_types(self, root: ET.Element):
        # Handle <types> root or direct children
        types_node = root.find('types') if root.tag != 'types' else root
        if types_node is None:
            types_node = root

        # Iterate over modules and direct definitions
        for item in types_node.iter():
            if item.tag == 'typedef':
                name = item.get('name')
                # Prefer nonBasicTypeName, fallback to type
                type_val = item.get('nonBasicTypeName') or item.get('type')
                if name and type_val:
                    self.registry.register_typedef(name, type_val)
            
            elif item.tag == 'enum':
                name = item.get('name')
                if name:
                    values = []
                    for child in item:
                        if child.tag in ('enumerator', 'value', 'literal'):
                            val_name = child.get('name')
                            if val_name:
                                values.append(val_name)
                    self.registry.register_enum(name, values)
            
            elif item.tag == 'struct':
                name = item.get('name')
                if name:
                    self.registry.register_struct(name, item)

class JsonGenerator:
    def __init__(self, registry: TypeRegistry):
        self.registry = registry

    def generate_sample(self, type_name: str, is_sequence: bool = False) -> Any:
        if is_sequence:
            # Generate a list with one element
            return [self.generate_sample(type_name, False)]

        # Resolve type to handle typedefs
        resolved_type = self.registry.resolve_type(type_name)
        
        # 1. Check if it's an Enum
        enum_values = self.registry.get_enum_values(resolved_type)
        if enum_values:
            return enum_values[0] if enum_values else "UNKNOWN_ENUM"

        # 2. Check if it's a Struct
        struct_def = self.registry.get_struct(resolved_type)
        if struct_def is not None:
            return self._generate_struct_sample(struct_def)

        # 3. Primitives
        # Check for basic types (including those from typedefs like T_Int32 -> long)
        lower_type = resolved_type.lower()
        
        if 'boolean' in lower_type or 'bool' in lower_type:
            return False
        if 'float' in lower_type or 'double' in lower_type:
            return 1.0
        if 'int' in lower_type or 'long' in lower_type or 'short' in lower_type or 'byte' in lower_type:
            return 1
        if 'char' in lower_type or 'string' in lower_type:
            return "string"
        
        # Fallback
        return f"UnknownType({type_name})"

    def _generate_struct_sample(self, struct_element: ET.Element) -> Dict[str, Any]:
        result = {}
        
        # Handle inheritance if needed (baseType) - simplified for now as not explicitly requested but good to have
        base_type = struct_element.get('baseType')
        if base_type:
            base_sample = self.generate_sample(base_type)
            if isinstance(base_sample, dict):
                result.update(base_sample)

        for member in struct_element:
            if member.tag not in ('member', 'field', 'element'):
                continue
            
            name = member.get('name')
            if not name:
                continue

            # Determine Type
            type_name = member.get('nonBasicTypeName') or member.get('type')
            
            # Determine if Sequence/Array
            is_sequence = False
            if member.get('sequenceMaxLength') or (member.get('type') and 'sequence' in member.get('type')):
                is_sequence = True
            
            # Generate Value
            value = self.generate_sample(type_name, is_sequence)
            
            # Special Rule: String value with A_ prefix stripping
            # If the value is a string and not an enum value (enums are strings in JSON usually)
            # We need to check if it was a primitive string generation.
            # A simple heuristic: if the value is "string" (from primitive check) or we want to use the field name.
            # The C# logic says: if string, use field name (stripped of A_).
            
            # Let's refine the primitive string generation.
            # If generate_sample returned "string", we replace it with the field name logic.
            if value == "string":
                clean_name = name
                if clean_name.startswith("A_"):
                    clean_name = clean_name[2:]
                value = clean_name

            result[name] = value
            
        return result

def main():
    import argparse

    parser = argparse.ArgumentParser(description="Generate Sample JSON from DDS XML definitions.")
    parser.add_argument("input_files", nargs='+', help="Input XML file paths (one or more)")
    parser.add_argument("-o", "--output", default="output_json", help="Output directory for JSON files")
    
    args = parser.parse_args()

    # Expand wildcards manually for shells that don't support globbing (e.g. Windows CMD/PowerShell)
    input_files = []
    for pattern in args.input_files:
        if any(char in pattern for char in ['*', '?', '[']):
            matched = glob.glob(pattern)
            if not matched:
                print(f"Warning: No files matched pattern '{pattern}'")
            input_files.extend(matched)
        else:
            input_files.append(pattern)

    output_dir = args.output

    # 1. Initialize Registry
    registry = TypeRegistry()
    schema_parser = SchemaParser(registry)

    # 2. Scan directories of all input files for dependencies
    #    (We scan all unique directories involved to ensure we have all types)
    scanned_dirs = set()
    for xml_path in input_files:
        if not os.path.exists(xml_path):
            print(f"Error: Input file {xml_path} not found.")
            continue
            
        abs_path = os.path.abspath(xml_path)
        directory = os.path.dirname(abs_path)
        
        if directory not in scanned_dirs:
            print(f"Scanning directory for types: {directory}...")
            schema_parser.parse_directory(directory)
            scanned_dirs.add(directory)

    # Ensure output directory exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    generator = JsonGenerator(registry)
    total_count = 0

    # 3. Process each target file to generate JSONs
    for xml_path in input_files:
        if not os.path.exists(xml_path):
            continue

        print(f"Processing target file: {xml_path}")
        try:
            tree = ET.parse(xml_path)
            root = tree.getroot()
        except ET.ParseError as e:
            print(f"Error parsing target file {xml_path}: {e}")
            continue

        # Find all structs in the target file
        types_node = root.find('types') if root.tag != 'types' else root
        if types_node is None:
            types_node = root

        file_count = 0
        for module_name, struct_element in iter_structs_with_module(types_node):
            struct_name = struct_element.get('name')
            if not struct_name or not struct_name.upper().startswith('C_'):
                continue

            sample_data = generator.generate_sample(struct_name)
            logical_name = f"{module_name}::{struct_name}" if module_name else struct_name
            file_basename = sanitize_filename(logical_name.replace('::', '__'))
            output_file = os.path.join(output_dir, f"{file_basename}.json")

            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(sample_data, f, indent=2, ensure_ascii=False)

            file_count += 1
            print(f"    {logical_name} -> {os.path.basename(output_file)}")

        print(f"  -> Generated {file_count} JSON files.")
        total_count += file_count

    print(f"Done. Total {total_count} JSON files generated in '{output_dir}'.")

if __name__ == "__main__":
    main()
