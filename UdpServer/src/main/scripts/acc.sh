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

MAIN_CLASS="com.sjwyb.AccessStatistic"

for f in $LIB/*.jar; do
  CLASSPATH=${CLASSPATH}:$f
done

JVM_OPTS="-server -Xms256m -Xmx256m"

find -L $HOME | grep access.log | xargs cat | \
  java $JVM_OPTS -cp $CLASSPATH $MAIN_CLASS $@