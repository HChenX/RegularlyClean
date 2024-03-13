outPut() {
  echo "$@"
  sleep 0.05
}

path="/sdcard/Android/RegularlyClean"
path_old="/sdcard/Android/RegularlyClean_old"
crond_path="$path/ScheduledTasks"

[[ -d $path ]] && {
  outPut "杀死正在运行的crond进程···"
  crond_pid="$(pgrep -f 'regularly.d' | grep -v $$)"
  [[ -n "$crond_pid" ]] && {
    for kill_pid in $crond_pid; do
      kill -9 "$kill_pid" && outPut "杀死crond进程: $kill_pid"
    done
  }
  outPut "备份历史文件···"
  rm -rf "$path_old" && mkdir -p "$path_old"
  cp -rf $path/* "$path_old"
  rm -rf "$path"
  outPut "历史文件路径：$path_old"
}

mkdir -p $crond_path
cp -r "$MODPATH"/AndroidFile/Blacklist.prop $path/
cp -r "$MODPATH"/AndroidFile/Whitelist.prop $path/
cp -r "$MODPATH"/AndroidFile/ModuleConfig.ini $path/
cp -r "$MODPATH"/AndroidFile/ScheduledTasks/CreateCrond.sh $crond_path/
rm -rf "$MODPATH"/AndroidFile/
outPut "安装完成！"