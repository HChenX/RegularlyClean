module="/data/adb/modules/crond_clear_the_blacklist"
moduleksu="/data/adb/ksu/modules/crond_clear_the_blacklist"
{ [[ -d "$module" ]] && { mod_path=$module; }; } || { { [[ -d "$moduleksu" ]] && mod_path=$moduleksu; }; } || { echo "- [!]:不存在主模块文件，请安装模块" && exit 88; }
{ [[ -f "/data/adb/ksud" ]] && { alias crond="/data/adb/busybox/crond"; }; } || { alias crond="\$( magisk --path )/.magisk/busybox/crond"; }
set_path=${0%/*} && set_file=$set_path/定时设置.ini && cron_d_path=$mod_path/script/set_cron.d
[[ ! -d $cron_d_path ]] && mkdir -p $cron_d_path
. $mod_path/script/clear_the_blacklist_functions.sh
{ [[ -f $set_file ]] && { { . "$set_file" && { echo "- [i]:文件读取成功"; }; } || { echo "- [!]:文件读取异常，请审查(设置定时.ini)文件内容！" && exit 1; }; }; } || { echo "- [!]:缺少$set_file文件" && exit 2; }
case $cron in
y | n) echo "- [i]:填写正确 | cron=$cron" ;;
*) echo "- [!]:填写错误 | cron=$cron | 请重新填写" && exit 1 ;;
esac
main() {
  other_mod() {
    case $bigfile_auto_skip in
    y | n) echo "- [i]:填写正确 | bigfile_auto_skip=$bigfile_auto_skip" ;;
    *) echo "- [!]:填写错误 | bigfile_auto_skip=$bigfile_auto_skip | 请填写y或n" && exit 12 ;;
    esac
    [[ $bigfile_auto_skip == y ]] && { { [[ $bigfile_mb -gt 0 ]] && { echo "- [i]:填写正确 | bigfile_mb=$bigfile_mb"; }; } || { echo "- [!]:填写错误 | bigfile_mb=$bigfile_mb | 请大于0" && exit 13; }; }
    case $clear_mod in
    y | n) echo "- [i]:填写正确 | clear_mod=$clear_mod" ;;
    *) echo "- [!]:填写错误 | clear_mod=$clear_mod | 请填写y或n" && exit 12 ;;
    esac
    [[ $clear_mod == y ]] && { { [[ $sleep -gt 0 ]] && [[ $sleep -lt 6 ]] && echo "- [i]:填写正确 | sleep=$sleep"; } || { echo "- [!]:填写错误 | sleep=$sleep | 范围1-5" && exit 14; }; }

  }
  { [[ $cron == y ]] && {
    open_value() {
      mod_1=$(echo "$open_mod" | grep "1")
      mod_2=$(echo "$open_mod" | grep "2")
      mod_3=$(echo "$open_mod" | grep "3")
      minute_mod() {
        { [[ $mod_1 != "" ]] && {
          case $minute in
          [0-9]*) { [[ $minute -ge 1 ]] && [[ $minute -le 60 ]] && { echo "- [i]:填写正确 | minute=$minute"; }; } || { echo "- [!]:填写错误 | minute=$minute | 请重新填写" && exit 3; } ;;
          esac
          minute_out=$minute && minute="*/$minute"
        }; } || { minute="*"; }
      }
      time_mod() {
        { [[ $mod_2 != "" ]] && { {
          { ! echo "$what_time" | grep -q "-" && minute="0" && minute_out=$minute && { time_out=$what_time && what_time="*/$what_time"; }; } || {
            time_1=$(echo "$what_time" | awk -F "-" '{print $1}')
            time_2=$(echo "$what_time" | awk -F "-" '{print $2}')
            { [[ -z $time_1 ]] || [[ -z $time_2 ]] && { echo "- [!]: 填写错误 | what_time=$what_time" && exit 3; }; } || { { ! echo "$time_1" | grep -q "[0-9]"; } || { ! echo "$time_2" | grep -q "[0-9]"; } && { echo "- [!]: 填写错误 | what_time=$what_time | 填写数字" && exit 4; }; } || { { [[ ${#time_1} -ge 3 ]]; } || { [[ ${#time_2} -ge 3 ]]; } && { echo "- [!]: 填写错误 | what_time=$what_time | 不要超过3个字符" && exit 5; }; } || { { [[ $time_1 == "$time_2" ]]; } && { echo "- [!]: 填写错误 | what_time=$what_time | 时间不能相同" && exit 6; }; } || { { [[ $time_1 -ge 24 ]]; } || { [[ $time_2 -ge 24 ]]; } && { echo "- [!]: 填写错误 | what_time=$what_time | 时间不能大于或等于24点 这里的24点是0点" && exit 7; }; } || { echo "- [i]:填写正确 | what_time=$what_time"; }
          }
        }; }; } || { what_time="*"; }
      }
      day_mod() {
        { [[ $mod_3 != "" ]] && {
          { { [[ -z $what_day ]]; } && { echo "- [!]: 填写错误 | what_day=$what_day | 格式: 数字1-31" && exit 8; }; } || { { ! echo "$what_day" | grep -q "[0-9]"; } && { echo "- [!]: 填写错误 | what_day=$what_day | 请填写数字！" && exit 9; }; } || { { [[ $what_day -gt 31 ]] || [[ $what_day -lt 1 ]]; } && { echo "- [!]: 填写错误 | what_day=$what_day | 范围限制：1-31" && exit 10; }; } || {
            echo "- [i]:填写正确 | what_day=$what_day" && day_out=$what_day && what_day="*/$what_day"
          }
        }; } || { what_day="*"; }
      }
      print_name() {
        echo "- [i]:内容解读 | $1$2$3运行一次。"
        echo "$1$2$3运行一次。" >$mod_path/print_set
      }
      print_out() {
        echo "- [i]:定时设置 | $crond_rule"
        print_name "$day_print" "$time_print" "$minute_print"
        echo -n "$crond_rule" >$mod_path/script/Crond_time
      }
      set_cron() {
        { [[ $minute == "*" ]] && minute_out="未定义"; } || { [[ $minute != "0" ]] && minute_print="每隔$minute_out分钟，"; }
        { [[ $what_time == "*" ]] && time_out="未定义"; } || { [[ $minute != "0" ]] && {
          [[ $time_1 -gt $time_2 ]] && cn_text="第二天" || cn_text=""
          [[ $time_2 == 0 ]] && cn_text=""
          time_print="$time_1:00到$cn_text$time_2:59，"
        }; }
        [[ $minute == "0" ]] && time_print="每隔$time_out小时，"
        { [[ $what_day == "*" ]] && day_out="未定义"; } || { day_print="每隔$day_out天，"; }
        logd_ini="minute=$minute_out | what_time=$time_out | what_day=$day_out"
        crond_rule="$minute $what_time $what_day * *"
        print_out
      }
      app_clear() {
        [[ $clear_mod == y ]] && {
          pid() { clear_pid="$(pgrep -f 'Clearapp.sh' | grep -v $$)"; }
          pid
          [[ -z $clear_pid ]] && {
            sh $module/clear.sh &>/dev/null
            pid
            echo "- [i]:启动监测 | pid：$clear_pid" && echo "- [i]:重要提示 | 现在可以退出脚本了"
          } || { echo "- [i]:启动监测 | pid：$clear_pid"; }
        }
      }
      kill_pid() {
        crond_pid="$(pgrep -f 'set_cron.d' | grep -v $$)"
        [[ -n $crond_pid ]] && {
          for i in $crond_pid; do
            echo "- [i]:杀死定时 | pid: $i"
            kill -9 "$i"
          done
        }
      }
      cron_on() {
        echo "$crond_rule $mod_path/script/Run_clear.sh" >$cron_d_path/root && crond -c "$cron_d_path"
        crond_pid_1="$(pgrep -f 'set_cron.d' | grep -v $$)"
        { [[ $crond_pid_1 == "" ]] && { echo "- [!]:定时启动失败，运行失败" && exit 11; }; } || {
          echo "- [i]:定时启动 | pid: $crond_pid_1" && log_md_set_cron_clear
          { [[ -f $mod_path/script/Run_clear.sh ]] && { sh $mod_path/script/Run_clear.sh >/dev/null; }; } || { kill -9 "$crond_pid_1" && echo "- [!]:模块脚本缺失！" && echo "- [!]:已终止定时！" && logd "- [!]:模块脚本缺失！" && logd "- [!]:已终止定时！"; }
        }
      }
      minute_mod
      time_mod
      day_mod
      other_mod
      set_cron
      app_clear
      kill_pid
      cron_on
    }
    open_value
  }; } || { echo "- [!]:未开启定时，退出设置" && exit 99; }
}
main
