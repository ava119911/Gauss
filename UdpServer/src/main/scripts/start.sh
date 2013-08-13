#!/usr/bin/env bash

PRG="$0"

while [ -h "$PRG" ]; do
  ls=`ls -ld "$PRG"`
  link=`expr "$ls" : '.*-> \(.*\)$'`
  if expr "$link" : '/.*' > /dev/null; then
    PRG="$link"
  else
    PRG=`dirname "$PRG"`/"$link"
  fi  
done

HOME=`cd $(dirname "$PRG"); pwd`
LIB=$HOME/lib
LOGDIR=$HOME

if [ ! -d $LOGDIR ]; then
    mkdir -p "$LOGDIR"
fi

MAIN_CLASS="com.sjwyb.UdpServer"

for f in $LIB/*.jar; do
	CLASSPATH=${CLASSPATH}:$f
done

JVM_OPTS="-server -Xms256m -Xmx256m"

nohup java $JVM_OPTS -Dlogdir="$LOGDIR" -cp $CLASSPATH $MAIN_CLASS >"$LOGDIR/stdout.log" 2>&1 &

echo "udpserver started"

echo $! > udpserver.pid
