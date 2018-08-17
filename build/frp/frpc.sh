#! /bin/sh

while true;
do
    PID=`ps |grep 'frp/frpc'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'`
    echo $PID >PID
    if [ "$PID">0 ]; then
          ./frp/autorun.sh
    fi
    sleep 10
done