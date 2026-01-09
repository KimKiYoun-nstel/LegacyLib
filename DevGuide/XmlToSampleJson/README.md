# XmlToSampleJson

설명

`XmlToSampleJson`는 DDS XML 타입 정의에서 `C_` 접두사의 구조체를 찾아 샘플 JSON 파일을 생성하는 Python 도구입니다. 이 저장소에 이미 포함되어 있으며, Windows PowerShell 환경에서도 와일드카드 패턴을 지원하도록 구현되어 있습니다.

주요 파일

- `generate_sample_json.py` : 파서 및 JSON 생성 스크립트

요구사항

- Python 3.8 이상 (표준 라이브러리만 사용되어 별도 종속 패키지 설치 불필요)

사용법(예시, PowerShell)
```powershell
# 단일 파일
python Tools/XmlToSampleJson/generate_sample_json.py config/generated/Nstel_PSM.xml -o output_json

# 다중 파일(와일드카드 지원)
python Tools/XmlToSampleJson/generate_sample_json.py config/generated/*.xml -o output_json
```

출력 파일명 규칙

- 생성되는 파일의 논리 이름은 `모듈명::구조체명`입니다. 파일 시스템에서 허용되지 않는 문자는 `_`로 치환되어 저장됩니다.

예: `P_NSTEL::C_Vehicle_Speed` -> `P_NSTEL__C_Vehicle_Speed.json`

주의사항

- 입력 XML들이 참조하는 다른 XML(예: `LDM_Common.xml`)이 같은 디렉토리에 있어야 정상적으로 타입을 해석합니다. 필요시 추가 디렉토리를 스캔하도록 스크립트를 확장할 수 있습니다.
