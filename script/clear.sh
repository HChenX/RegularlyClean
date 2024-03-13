#全局变量
MODDIR=${0%/*}
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
      com_xiaomi_market "$i"
      [[ $? == 2 ]] && continue
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

#拉起子sh进程
. "$MODDIR/clear_the_blacklist_functions.sh"

#读取配置文件
conf="$black_and_white_list_path"/定时任务/定时设置.ini
{
  [[ -f $conf ]] && {
    . "$conf" || {
      logd "- [!]: 文件读取异常，请审查(设置定时.ini)文件内容！" && exit 1
    }
  }
} || {
  logd "- [!]: 缺少$conf文件" && exit 2
}

#获取配置
{
  [[ $bigfile_auto_skip == y ]] && {
    skip_mb="$bigfile_mb"
  }
} || {
  skip_mb=-1
}

#拉起模组进程
. "$MODDIR/Run_clear_mod.sh"

{
  [[ "$Screen" == "亮屏" ]] && {
    logd "[状态]: [$Screen] 执行"
    #创建白名单拦截次数记录文件
    [[ ! -d $MODDIR/White_List_File ]] && mkdir -p "$MODDIR"/White_List_File
    #创捷记录文件
    [[ ! -d $MODDIR/DATE ]] && mkdir -p "$MODDIR"/DATE
    tmp_date="$MODDIR/DATE/$(date '+%Y%m%d')"
    #判断是否是新的一天
    [[ ! -d "$tmp_date" ]] && {
      rm -rf "$MODDIR"/DATE &>/dev/null
      mkdir -p "$tmp_date"
      echo "0" >"$tmp_date"/file
      echo "0" >"$tmp_date"/dir
      log_md_clear
    }
    #向主程序传入已有的记录
    FILE="$(cat "$tmp_date"/file)"
    DIR="$(cat "$tmp_date"/dir)"
    #读取黑白名单
    White_List_Expand="$(whitelist_wildcard_list)"
    Black_List_Expand="$(blacklist_wildcard_list)"
    # 执行方法
    main_for

    #读取清理记录显示在模块界面
    FILE="$(cat "$tmp_date"/file)"
    DIR="$(cat "$tmp_date"/dir)"
    sed -i "/^description=/c description=CROND: [ 今日已清除: $FILE个黑名单文件 | $DIR个黑名单文件夹 ]" "${MODDIR%/script}/module.prop"
  }
} || {
  logd "[状态]: [$Screen] 不执行"
}
