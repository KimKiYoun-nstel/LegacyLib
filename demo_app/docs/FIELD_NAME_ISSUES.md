 # 필드명 오타 및 불일치 기록

이 파일은 현재 코드와 문서에서 사용되는 필드명 중 오타, 언더스코어(_ ) 사용의 불일치, 스키마와 구현 간 차이로 보이는 항목들을 정리한 문서입니다. 인터페이스 규격이 공식적으로 변경될 때까지 참고용으로 보관하시고, 규격 변경 시 코드와 문서를 함께 갱신해야 합니다.

검토된 항목(메시지에 실제 사용된 정확한 문자열 보존):

- `A_topForwardGryro` — "Gryro" 오타 (올바른 이름: `A_topForwardGyro`). PBIT/CBIT에 사용됨.
- `A_vehicleForwardGyroi` — 끝에 불필요한 'i' 포함 (올바른 이름: `A_vehicleForwardGyro`). PBIT/CBIT에 사용됨.
- `A_manualArmPositionComple` — "Comple"로 잘림 (올바른 이름: `A_manualArmPositionComplement`). Actuator Signal에 사용됨.
- `A_power_Controller` — resultBIT 스키마에서 언더스코어 사용(`A_power_Controller`), 코드/문서의 다른 위치에서는 `A_powerController` 형태와 불일치.
- `A_round_Park` — 언더스코어 사용의 불일치 (통일된 표기 예: `A_roundPark`).
- `A_mainCannonRestoreComplement` 등 일부 필드명의 일관성(예: "RestoreComplement" 표기)을 재검토 필요.

현재 코드베이스의 결정 사항:
- 기존 동작과의 호환성을 위해 JSON 생성 시 코드에서는 현재 사용 중인 필드 문자열을 그대로 유지합니다. 즉, AgentUI와의 현재 연동을 깨지 않습니다.
- 필드명 변경은 공식 규격 변경 요청이 있을 때까지 보류합니다. 규격이 변경되면 `demo_msg_publish_*` 함수들과 파싱 로직을 함께 수정해야 합니다.

추가 제안:
- 향후 변경을 용이하게 하기 위해 메시지 필드명을 헤더나 상수로 중앙화(single source of truth)하면 추후 리네임 시 수정 범위를 줄일 수 있습니다. 원하시면 해당 패치도 만들어 드립니다.

