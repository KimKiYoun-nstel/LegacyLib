# 필드명 레퍼런스 및 규칙

이 문서는 현재 코드베이스에서 사용하는 최종 필드명 규칙과 과거에 발견된 불일치 항목의 정리본입니다. 모든 메시지 필드명은 `demo_app/include/msg_fields.h`에 정의된 문자열을 1차 소스(Single Source of Truth)로 사용합니다.

핵심 규칙
---------
- 중앙 정의: `demo_app/include/msg_fields.h`에 정의된 문자열을 사용합니다.
- 시간 필드: `A_timeOfDataGeneration` 내부는 `A_seconds` 사용 (예: `"A_timeOfDataGeneration": { "A_seconds": 123456789 }`).
- 소스 ID: `A_sourceID` 내부는 `A_systemID`, `A_unitID` 등으로 구성됩니다.
- 자이로 표기: `Giro` 표기(`A_roundGiro`, `A_upDownGiro`)로 통일합니다.
- 열거자 표기: 스키마 접두사 포함 (예: `L_OperationModeType_EMERGENCY`, `L_BITResultType_NORMAL`).

과거에 발견된 항목(현재는 수정 완료)
- `A_upDownGyro` → `A_upDownGiro`: 명칭 통일
- `EMER_GENCY`(타이포) → `L_OperationModeType_EMERGENCY`: 열거자 문자열 정정
- `A_timeOfDataGeneration.seconds` → `A_timeOfDataGeneration.A_seconds`: 중첩 필드명 통일

검증 권고
- 메시지 생산자/소비자는 런타임에서 필드 존재 여부와 타입을 `nlohmann::json`으로 엄격히 검사할 것.
- 필드명 변경 시 `msg_fields.h`와 문서들을 동시에 갱신할 것.

필요 시 저는 이 파일을 기준으로 코드베이스 전체에서 사용하는 필드명을 검증하고, 자동 치환 패치를 생성해 드릴 수 있습니다.
