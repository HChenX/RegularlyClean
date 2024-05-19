outPut() {
  echo "$@"
  sleep 0.05
}

path="/data/media/0/Android/RegularlyClean"
path_old="/data/media/0/Android/RegularlyClean_old"

[[ -d $path ]] && {
  outPut "杀死正在运行的crond进程···"
  crond_pid="$(pgrep -f 'RegularlyClean' | grep -v $$)"
  [[ -n "$crond_pid" ]] && {
    for kill_pid in $crond_pid; do
      kill -9 "$kill_pid" && outPut "杀死crond进程: $kill_pid"
    done
  }
  outPut "备份历史文件···"
  rm -rf "$path_old" && mkdir -p "$path_old"
  cp -rf $path/* "$path_old"
  chmod -R 0777 $path_old
  rm -rf "$path"
  outPut "历史文件路径：$path_old"
  outPut "更新模块会导致原规则和配置被还原！!"
  outPut "被还原配置包括: "
  outPut "blacklist.txt, whitelist.txt, config.ini"
  outPut "劳烦从备份文件复制到规则和配置文件内！！"
}

mkdir -p $path
cp -arf "$MODPATH"/AndroidFile/* $path/
rm -rf "$MODPATH"/AndroidFile/
outPut "安装完成！"
