 # 필드명 오타 및 불일치 기록

이 파일은 현재 코드와 문서에서 사용되는 필드명 중 오타, 언더스코어(_ ) 사용의 불일치, 스키마와 구현 간 차이로 보이는 항목들을 정리한 문서입니다. 인터페이스 규격이 공식적으로 변경될 때까지 참고용으로 보관하시고, 규격 변경 시 코드와 문서를 함께 갱신해야 합니다.

검토된 항목(표준화된 필드명):

- `A_topForwardGiro` — 상단 전방 Giro (canonical)
- `A_vehicleForwardGiro` — 차량 전방 Giro (canonical)
- `A_manualArmPositionComplement` — Actuator Signal의 완전한 이름
- `A_powerController` — resultBIT에서 언더스코어 제거된 표기 (canonical)
- `A_roundPark` — 언더스코어 제거된 표기 (canonical)

현재 코드베이스의 결정 사항:
- 문서와 코드 모두 RefDoc XML 스키마에 맞춰 필드명을 표준화했습니다.
- 향후 규격 변경 시 중앙화된 필드 매크로(`msg_fields.h`)를 사용해 일괄 수정할 수 있습니다.

추가 제안:
- 향후 변경을 용이하게 하기 위해 메시지 필드명을 헤더나 상수로 중앙화(single source of truth)하면 추후 리네임 시 수정 범위를 줄일 수 있습니다. 원하시면 해당 패치도 만들어 드립니다.

