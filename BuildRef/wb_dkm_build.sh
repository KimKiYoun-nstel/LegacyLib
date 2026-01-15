#!/bin/sh
set -eu

echo "[wb_dkm_build] args: $*"

# script is in <proj>/BuildRef
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Workbench builder working dir (폴더 선택 빌드 시 달라짐)
WD="$(pwd)"
WD_BASE="$(basename "$WD")"

DO_CLEAN=0
TRACE=0
for a in "$@"; do
  [ "$a" = "clean" ] && DO_CLEAN=1
  case "$a" in TRACE=*) TRACE="${a#TRACE=}" ;; esac
done

echo "[wb_dkm_build] PROJ_DIR=$PROJ_DIR"
echo "[wb_dkm_build] WD=$WD (base=$WD_BASE)"
echo "[wb_dkm_build] DO_CLEAN=$DO_CLEAN"

# Convert D:/... -> D:\...  (cmd.exe가 알아먹게)
to_win_path() { printf '%s' "$1" | sed 's#/#\\#g'; }

PROJ_DIR_WIN="$(to_win_path "$PROJ_DIR")"
WD_WIN="$(to_win_path "$WD")"

# 임시 cmd는 "현재 WD"에 만들면 Workbench 권한/경로 이슈가 제일 적음
TMP_CMD="$WD_WIN\\wb_dkm_run.cmd"

# 실행할 커맨드 결정
# (tasks.json 정의를 그대로 사용) :contentReference[oaicite:5]{index=5}
if [ "$DO_CLEAN" -eq 1 ]; then
  ACTIONS='make -C legacy_lib clean MODE=vxworks && make -C demo_app clean MODE=vxworks'
else
  case "$WD_BASE" in
    legacy_lib)
      ACTIONS='call BuildRef\\build_vx_lib.cmd'
      ;;
    demo_app)
      ACTIONS='call BuildRef\\build_vx_demo.cmd'
      ;;
    *)
      # 프로젝트 전체 Build: lib -> demo
      ACTIONS='call BuildRef\\build_vx_lib.cmd && call BuildRef\\build_vx_demo.cmd'
      ;;
  esac
fi

# cmd 파일 생성
{
  echo "@echo off"
  echo "setlocal"
  echo "cd /d \"$PROJ_DIR_WIN\""
  echo "call BuildRef\\vx_set_env.cmd"
  echo "$ACTIONS"
  echo "set ERR=%ERRORLEVEL%"
  echo "endlocal & exit /b %ERR%"
} > "$TMP_CMD"

if [ "$TRACE" = "1" ]; then
  echo "[wb_dkm_build] CMD FILE = $TMP_CMD"
fi

# 실행 (따옴표/이스케이프 꼬임 방지: 경로만 넘김)
cmd /c "$TMP_CMD"
RET=$?

# 정리 (best-effort)
rm -f "$WD/wb_dkm_run.cmd" 2>/dev/null || true

exit $RET
