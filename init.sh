MODDIR=${0%/*}

. "$MODDIR"/utils.sh

logd "初始化完成！"

{
  [[ -f $crondFile ]] && {
    crond -c $crondPath
  }
} || {
  echo "*/10 7-23 * * * $MODDIR/data/clear.sh" >$crondFile
  echo -n "*/10 7-23 * * *" >$MODDIR/data/crond_data
  crond -c $crondPath
}

{
  [[ $(pgrep -f 'regularly.d' | grep -v $$) != "" ]] && {
    device
    logd "开始运行了！"
    logd "------------------------------------------------------------"
  }
} || {
  device
  logd "运行失败！"
  exit 1
}
