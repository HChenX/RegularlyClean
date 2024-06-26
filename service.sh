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

. "$MODDIR"/utils.sh
chmod -R 0777 "$MODDIR"
chmod -R 0777 "$path"
echo -n "$MODDIR" >"$path"/module_path
logClear "开机启动成功！"
killAll
sh "$MODDIR"/init.sh
"$MODDIR"/RegularlyClean &