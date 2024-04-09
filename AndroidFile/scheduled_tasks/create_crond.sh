path=${0%/*}
path=$(dirname "$path")
{
  [[ -f $path/module_path ]] && {
    module_path=$(cat $path/module_path)
  }
} || {
  echo "-[!]: 未安装主模块！"
  exit 1
}
MODDIR=$module_path
. $module_path/utils.sh
config=$path/config.ini

[[ ! -d $crondPath ]] && mkdir -p $crondPath
{
  [[ -f $config ]] && {
    {
      . "$config" && {
        echo "- [i]: 文件读取成功"
      }
    } || {
      echo "- [!]: 文件读取异常，请审查(config.ini)文件内容！" && exit 1
    }
  }
} || {
  echo "- [!]: 缺少$config文件" && exit 1
}

main() {
  case $bigfile_auto_skip in
  y | n) echo "- [i]: 填写正确 | bigfile_auto_skip=$bigfile_auto_skip" ;;
  *) echo "- [!]: 填写错误 | bigfile_auto_skip=$bigfile_auto_skip | 请填写y或n" && exit 1 ;;
  esac
  [[ $bigfile_auto_skip == y ]] && {
    {
      [[ $bigfile_mb -gt 0 ]] && {
        echo "- [i]: 填写正确 | bigfile_mb=$bigfile_mb"
      }
    } || {
      echo "- [!]: 填写错误 | bigfile_mb=$bigfile_mb | 请大于0" && exit 1
    }
  }
  case $clear_mod in
  y | n) echo "- [i]: 填写正确 | clear_mod=$clear_mod" ;;
  *) echo "- [!]: 填写错误 | clear_mod=$clear_mod | 请填写y或n" && exit 1 ;;
  esac
  open_value
}
open_value() {
  minute_mod() {
    {
      [[ $minute != "" ]] && {
        case $minute in
        [0-9]*) {
          [[ $minute -ge 1 ]] && [[ $minute -le 60 ]] && {
            echo "- [i]: 填写正确 | minute=$minute"
          }
        } || {
          echo "- [!]: 填写错误 | minute=$minute | 请重新填写" && exit 1
        } ;;
        esac
        minute="*/$minute"
      }
    } || {
      minute="*"
    }
  }
  time_mod() {
    {
      [[ $what_time != "" ]] && {
        {
          {
            ! echo "$what_time" | grep -q "-" && minute="0" && {
              what_time="*/$what_time"
            }
          } || {
            time_1=$(echo "$what_time" | awk -F "-" '{print $1}')
            time_2=$(echo "$what_time" | awk -F "-" '{print $2}')
            {
              [[ -z $time_1 ]] || [[ -z $time_2 ]] && {
                echo "- [!]: 填写错误 | what_time=$what_time" && exit 1
              }
            } || {
              {
                ! echo "$time_1" | grep -q "[0-9]"
              } || {
                ! echo "$time_2" | grep -q "[0-9]"
              } && {
                echo "- [!]: 填写错误 | what_time=$what_time | 填写数字" && exit 1
              }
            } || {
              {
                [[ ${#time_1} -ge 3 ]]
              } || {
                [[ ${#time_2} -ge 3 ]]
              } && {
                echo "- [!]: 填写错误 | what_time=$what_time | 不要超过3个字符" && exit 1
              }
            } || {
              {
                [[ $time_1 == "$time_2" ]]
              } && {
                echo "- [!]: 填写错误 | what_time=$what_time | 时间不能相同" && exit 1
              }
            } || {
              {
                [[ $time_1 -ge 24 ]]
              } || {
                [[ $time_2 -ge 24 ]]
              } && {
                echo "- [!]: 填写错误 | what_time=$what_time | 时间不能大于或等于24点 这里的24点是0点" && exit 1
              }
            } || {
              echo "- [i]:填写正确 | what_time=$what_time"
            }
          }
        }
      }
    } || {
      what_time="*"
    }
  }
  day_mod() {
    {
      [[ $what_day != "" ]] && {
        {
          {
            [[ -z $what_day ]]
          } && {
            echo "- [!]: 填写错误 | what_day=$what_day | 格式: 数字1-31" && exit 1
          }
        } || {
          {
            ! echo "$what_day" | grep -q "[0-9]"
          } && {
            echo "- [!]: 填写错误 | what_day=$what_day | 请填写数字！" && exit 1
          }
        } || {
          {
            [[ $what_day -gt 31 ]] || [[ $what_day -lt 1 ]]
          } && {
            echo "- [!]: 填写错误 | what_day=$what_day | 范围限制：1-31" && exit 1
          }
        } || {
          echo "- [i]: 填写正确 | what_day=$what_day" && what_day="*/$what_day"
        }
      }
    } || {
      what_day="*"
    }
  }
  set_cron() {
    crond_rule="$minute $what_time $what_day * *"
    echo -n "$crond_rule" >"$module_path"/data/crond_data
  }
  kill_pid() {
    crond_pid="$(pgrep -f 'RegularlyClean' | grep -v $$)"
    [[ -n $crond_pid ]] && {
      for i in $crond_pid; do
        echo "- [i]: 杀死模块进程 | pid: $i"
        kill -9 "$i"
      done
    }
  }
  cron_on() {
    echo "$crond_rule $module_path/clear.sh" >$crondFile && crond -c "$crondPath"
    mPid="$(pgrep -f 'regularly.d' | grep -v $$)"
    {
      [[ $mPid == "" ]] && {
        echo "- [!]: 定时启动失败，运行失败" && exit 1
      }
    } || {
      echo "- [i]: 定时启动 | pid: $mPid" && logClear "成功修改配置，已经重新运行"
      {
        [[ -f $module_path/clear.sh ]] && {
          sh "$module_path"/clear.sh >/dev/null
          "$module_path"/RegularlyClean &
          echo "- [i]: 现在可以退出进程了"
          exit 0
        }
      } || {
        kill -9 "$mPid" && echo "- [!]: 模块脚本缺失！" && echo "- [!]: 已终止定时！" && logd "- [!]: 模块脚本缺失！" && logd "- [!]: 已终止定时！"
      }
    }
  }
  minute_mod
  time_mod
  day_mod
  set_cron
  kill_pid
  cron_on
}
main
