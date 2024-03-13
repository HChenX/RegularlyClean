# 跳过白名单次数判断
print_pd_white_list() {
  white_file="$MODDIR/White_List_File/$(echo "$1" | sed 's/\///g').ini"
  {
    [[ ! -e "$white_file" ]] && {
      echo "1" >"$white_file"
      logd "[continue] -[$(cat "$white_file")]- $2"
    }
  } || {
    # 小于3次则打印
    white_files="$(cat "$white_file")"
    [[ "$white_files" -lt "3" ]] && {
      echo "$((white_files + 1))" >"$white_file"
      logd "[continue] -[$(cat "$white_file")]- $2"
    }
  }
}
# 小米应用商店文件夹判断
com_xiaomi_market() {
  echo "$1" | grep -qw "com.xiaomi.market" && [[ "$(find "$1" -name "*.apk")" != "" ]] && {
    logd "存在APK: $1"
    return 2
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
        logd "[pass] -跳过：$bl"
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
