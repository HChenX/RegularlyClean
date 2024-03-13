MODDIR=${0%/*}

wait_login() {
  while [ "$(getprop sys.boot_completed)" != "1" ]; do
    sleep 1
  done

  while [ ! -d "/sdcard/Android" ]; do
    sleep 1
  done
}
wait_login
chmod -R 0777 "$MODDIR"
[[ -f "$MODDIR"/script/tmp/screen_state ]] && {
  rm -rf "$MODDIR"/script/tmp/screen_state
}
. "$MODDIR"/script/module_utils.sh
logd_clear "开机启动完成: [service.sh]"
sh "$MODDIR"/initial.sh