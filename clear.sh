#全局变量
MODDIR=${0%/*}

# 跳过白名单次数判断
print_pd_white_list() {
  white_file="$MODDIR/data/whitelist/$(echo "$1" | sed 's/\///g').ini"
  {
    [[ ! -e "$white_file" ]] && {
      echo "1" >"$white_file"
      logd "[skip] -[$(cat "$white_file")]- $2"
    }
  } || {
    # 小于3次则打印
    white_files="$(cat "$white_file")"
    [[ "$white_files" -lt "3" ]] && {
      echo "$((white_files + 1))" >"$white_file"
      logd "[skip] -[$(cat "$white_file")]- $2"
    }
  }
}

# 白名单列表通配符拓展
whitelist_wildcard_list() {
  local IFS=$'\n'
  for wh in $(grep -v '#' <"$White_List"); do
    [[ -n "$wh" ]] && {
      echo "$wh"
    }
  done
}

# 黑名单列表通配符拓展
blacklist_wildcard_list() {
  local IFS=$'\n'
  grep -v '#' <"$Black_List" | while read -r bl; do
    [[ -n "$bl" ]] && {
      [[ "${bl: -2}" == "/*" ]] && {
        logd "检测到/*，请替换成/&*"
        logd "[skip] -跳过：$bl"
        continue
      }
      #把/&*替换成/*
      [[ "${bl: -3}" == "/&*" ]] && {
        bl=${bl//\/&\*/\/\*}
      }
      [[ $skip_mb != -1 ]] && {
        big=$(find $bl -size +"$skip_mb"M 2>/dev/null)
        [[ $big != "" ]] && [[ -d $bl ]] && {
          {
            [[ "${bl: -1}" == "/" ]] && {
              bl="$bl*"
            }
          } || {
            [[ "${bl: -1}" != "*" ]] && {
              bl="$bl/*"
            }
          }
        }
      }
      for i in $bl; do
        {
          [[ $i == "$big" ]] && {
            logd "[big] -跳过：$i"
            continue
          }
        } || {
          echo "$i"
        }
      done
    }
  done
}

#主程序
main_for() {
  # 重新定义字段分隔符 忽略空格和制表符
  local IFS=$'\n'
  # 因为重新定义字段分隔符 所以只需要for 不再需要while read
  for i in $(echo "$Black_List_Expand" | grep -v -F '*'); do
    # 文件夹
    [[ -d "$i" ]] && {
      #跳过指定目录，防止危险
      case $i in
      *'/.') continue ;;
      *'/./') continue ;;
      *'/..') continue ;;
      *'/../') continue ;;
      esac
      echo "$White_List_Expand" | grep -qw "$i" && {
        print_pd_white_list "$i" "白名单DIR: $i"
        continue
      }
      rm -rf "$i" && {
        let DIR++
        logd "[rm] --黑名单DIR: $i"
        echo "$DIR" >"$tmp_date"/dir
      }
    }
    # 文件
    [[ -f "$i" ]] && {
      echo "$White_List_Expand" | grep -qw "$i" && {
        print_pd_white_list "$i" "白名单FILE: $i"
        continue
      }
      rm -rf "$i" && {
        let FILE++
        logd "[rm] --黑名单FILE: $i"
        echo "$FILE" >"$tmp_date"/file
      }
    }
  done
}

. "$MODDIR"/utils.sh
{
  [[ -f $config ]] && {
    . "$config" || {
      logd "- [!]: 文件读取异常，请审查(config.ini)文件内容！" && exit 1
    }
  }
} || {
  logd "- [!]: 缺少$config文件" && exit 2
}

Black_List="$path/blacklist.txt"
White_List="$path/whitelist.txt"
{
  [[ $bigfile_auto_skip == y ]] && {
    skip_mb="$bigfile_mb"
  }
} || {
  skip_mb=-1
}

{
  Screen_status="$(dumpsys window policy | grep 'mInputRestricted' | cut -d= -f2)"
  {
    [[ "$Screen_status" != "true" ]] && {
      Screen="亮屏"
    }
  } || {
    Screen="息屏"
  }

  [[ "$Screen" == "亮屏" ]] && {
    logd "[状态]: [$Screen] 执行"
    [[ ! -d $MODDIR/data/whitelist ]] && mkdir -p "$MODDIR"/data/whitelist
    [[ ! -d $MODDIR/data/date ]] && mkdir -p "$MODDIR"/data/date
    tmp_date="$MODDIR/data/date/$(date '+%Y%m%d')"
    [[ ! -d "$tmp_date" ]] && {
      rm -rf "$MODDIR"/data/date &>/dev/null
      mkdir -p "$tmp_date"
      echo "0" >"$tmp_date"/file
      echo "0" >"$tmp_date"/dir
      logNewDay
    }
    FILE="$(cat "$tmp_date"/file)"
    DIR="$(cat "$tmp_date"/dir)"
    White_List_Expand="$(whitelist_wildcard_list)"
    Black_List_Expand="$(blacklist_wildcard_list)"
    main_for

    FILE="$(cat "$tmp_date"/file)"
    DIR="$(cat "$tmp_date"/dir)"
    sed -i "/^description=/c description=CROND: [ 今日已清除: $FILE个黑名单文件 | $DIR个黑名单文件夹 ]" "$MODDIR/module.prop"
  }
} || {
  logd "[状态]: [$Screen] 不执行"
}
