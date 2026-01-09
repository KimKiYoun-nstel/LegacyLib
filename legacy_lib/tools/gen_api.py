import re
import os
import sys
import glob
import xml.etree.ElementTree as ET

def parse_xml_for_types(xml_dir):
    """
    XML 파일들을 파싱하여 (모듈명, 구조체명) 리스트를 반환합니다.
    """
    topics = []
    xml_files = glob.glob(os.path.join(xml_dir, "*.xml"))
    
    for xml_path in xml_files:
        try:
            tree = ET.parse(xml_path)
            root = tree.getroot()
            
            # RTI DDS XML schema defines types within <types>
            # Usually <module name="XXX"><struct name="YYY">
            for module in root.findall(".//module"):
                module_name = module.get("name")
                for struct in module.findall("struct"):
                    struct_name = struct.get("name")
                    if struct_name:
                        topics.append({
                            'module': module_name,
                            'name': struct_name,
                            'full_topic': f"{module_name}__{struct_name}",
                            'api_suffix': f"{module_name}_{struct_name}"
                        })
        except Exception as e:
            print(f"Error parsing {xml_path}: {e}")
            
    return topics

def parse_headers_for_structs(struct_dir):
    """
    헤더 파일들을 파싱하여 Wire_ 구조체 정보를 반환합니다.
    """
    wire_list = [] # List of {full, base, header, topic_str, type_name}
    header_files = glob.glob(os.path.join(struct_dir, "*.h"))
    
    for h_path in header_files:
        rel_h = os.path.basename(h_path)
        with open(h_path, 'r', encoding='utf-8') as f:
            content = f.read()
            # typedef struct Wire_XXX { ... } Wire_XXX;
            # Also capture the topic comment if possible: /** @brief Wire struct for P_NSTEL::C_... */
            matches = re.findall(r'/\*\*.*?Wire struct for ([\w:]+).*?\*/\s*typedef struct (Wire_(\w+))', content, re.DOTALL)
            for m in matches:
                canonical = m[0]
                topic_str = canonical.replace('::', '__')
                wire_list.append({
                    'full': m[1],
                    'base': m[2],
                    'header': rel_h,
                    'topic_str': topic_str,
                    'type_name': canonical
                })
            
            # Fallback for matches without descriptive comment
            if not matches:
                matches = re.findall(r'typedef struct (Wire_(\w+))', content)
                for m in matches:
                    # Check if already added
                    if any(w['full'] == m[0] for w in wire_list): continue
                    
                    # Guess topic/type
                    base_name = m[1] # e.g. P_NSTEL_C_VehicleSpeed
                    topic_str = base_name.replace('_', '__', 1) 
                    type_name = base_name.replace('_', '::', 1)
                    
                    wire_list.append({
                        'full': m[0],
                        'base': m[1],
                        'header': rel_h,
                        'topic_str': topic_str,
                        'type_name': type_name
                    })
                    
    return wire_list, [os.path.basename(h) for h in header_files]

def generate_api(struct_dir, xml_dir, out_dir):
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    topics = parse_xml_for_types(xml_dir)
    wire_list, headers = parse_headers_for_structs(struct_dir)

    header_file = os.path.join(out_dir, "legacy_api.h")
    source_file = os.path.join(out_dir, "legacy_api.c")

    # 1. Header Generation
    with open(header_file, 'w', encoding='utf-8') as f:
        f.write("/**\n * @file legacy_api.h\n * @brief 자동 생성된 API (JSON: XML 기반, Struct: 헤더 기반)\n */\n\n")
        f.write("#ifndef LEGACY_API_GENERATED_H\n")
        f.write("#define LEGACY_API_GENERATED_H\n\n")
        f.write("#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n")
        f.write("#include \"legacy_agent.h\"\n")
        
        for h in headers:
            f.write(f"#include \"{h}\"\n")
        f.write("\n")

        # JSON APIs (XML Driven)
        f.write("/* --- JSON API (Driven by XML) --- */\n")
        for t in topics:
            f.write(f"/** @brief Publish JSON for topic {t['full_topic']} (0x2000) */\n")
            f.write(f"int legacy_send_{t['api_suffix']}_json(void* agent, const char* json_data);\n\n")
        
        # Struct APIs (Header Driven)
        f.write("/* --- Struct API (Driven by Headers) --- */\n")
        for w in wire_list:
            f.write(f"/** @brief Publish Struct for {w['full']} (0x2100) */\n")
            f.write(f"int legacy_send_{w['base']}_struct(void* agent, const {w['full']}* data);\n\n")

        f.write("#ifdef __cplusplus\n}\n#endif\n")
        f.write("#endif // LEGACY_API_GENERATED_H\n")

    # 2. Source Generation
    with open(source_file, 'w', encoding='utf-8') as f:
        f.write("#include \"legacy_api.h\"\n")
        f.write("#include <string.h>\n\n")
        
        # JSON Implementation
        for t in topics:
            f.write(f"int legacy_send_{t['api_suffix']}_json(void* agent, const char* json_data) {{\n")
            f.write(f"    if (!agent || !json_data) return -1;\n")
            f.write(f"    return legacy_agent_publish_json(agent, \"{t['full_topic']}\", json_data);\n")
            f.write(f"}}\n\n")

        # Struct Implementation
        for w in wire_list:
            f.write(f"int legacy_send_{w['base']}_struct(void* agent, const {w['full']}* data) {{\n")
            f.write(f"    if (!agent || !data) return -1;\n")
            # Use extracted topic_str for consistent hashing with JSON/Control plane
            # Use type_name (canonical) for abi_hash matching
            f.write(f"    return legacy_agent_publish_struct(agent, \"{w['topic_str']}\", \"{w['type_name']}\", data, sizeof({w['full']}));\n")
            f.write(f"}}\n\n")

    print(f"Generated API: {len(topics)} JSON topics, {len(wire_list)} Struct types in {out_dir}")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python gen_api.py <struct_dir> <xml_dir> <out_dir>")
        sys.exit(1)
    
    generate_api(sys.argv[1], sys.argv[2], sys.argv[3])
