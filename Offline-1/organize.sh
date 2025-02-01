#!/bin/bash
submission=$1
targets=$2
tests=$3
answers=$4
verbose=0
noexecute=0
if [[ $* == *"-noexecute"* ]]; then
noexecute=1
fi
if [[ $* == *"-v"* ]]; then
verbose=1
fi


visit()
{
 if [[ -d $1 ]]; then
  for i in $1/*
    do
      visit "$i"
    done
 elif [[ -f $1 ]]; then
   if [[ "$1" == *.c || "$1" == *.py || "$1" == *.java ]]; then
  echo "$1"
   fi
 fi  
}



mkdir -p ./a $targets/C $targets/Python $targets/Java
cnt=$(find $tests -type f | wc -l)
if [[ $verbose -eq 1 ]]; then
echo "Found $cnt test files"
fi

for i in $submission/*
 do
   if [[ "$i" == *.zip ]] ; then
   name=${i%.zip}
   
   unzip -q "$i" -d ./a/${name: -7}/
  fi
done
if [[ $noexecute -eq 0 ]]; then
echo "student_id,type,matched,not_matched" >> $targets/result.csv
fi

for i in ./a/*
 do
   matched=0
  notMatched=0;
  if [[ $verbose -eq 1 ]]; then
 echo  "Organizing files of ${i: -7}"
  fi

  file=$(visit "$i")

  if [[ "$file" == *.c ]]; then
  
  mkdir -p $targets/C/${i: -7}/
  mv "$file" "$targets/C/${i: -7}/main.c"

  if [[ $noexecute -eq 0 ]]; then
   if [[ $verbose -eq 1 ]]; then
  echo  "Executing files of ${i: -7}"
  fi
  
  gcc -o $targets/C/${i: -7}/main.out "$targets/C/${i: -7}/main.c"
  type="C"
  j=1
   for input in $tests/*
    
    do 
    
    $targets/C/${i: -7}/main.out < $input   > "$targets/C/${i: -7}/out$j.txt"
    if  d=$(diff -q "$answers/ans$j.txt"  "$targets/$type/${i: -7}/out$j.txt") ; then
      ((matched++))
    else
      ((notMatched++))
    fi
    ((j++))
    
    done

  fi



 
  elif [[ "$file" == *.py ]]; then

  mkdir -p targets/Python/${i: -7}/
  mv "$file" "$targets/Python/${i: -7}/main.py"

  if [[ $noexecute -eq 0 ]]; then

    if [[ $verbose -eq 1 ]]; then
  echo  "Executing files of ${i: -7}"
  fi
 
  type="Python"
  j=1
   for input in $tests/*
    do 
    
    python3 "$targets/Python/${i: -7}/main.py" < $input  > "$targets/Python/${i: -7}/out$j.txt"
    if  d=$(diff -q "$answers/ans$j.txt"  "$targets/$type/${i: -7}/out$j.txt") ; then
      ((matched++))
    else
      ((notMatched++))
    fi
    ((j++))
    
    done

  fi

  elif [[ "$file" == *.java ]]; then
  type="Java"
  mkdir -p targets/Java/${i: -7}/
  mv "$file" "$targets/Java/${i: -7}/Main.java"

  if [[ $noexecute -eq 0 ]]; then
  if [[ $verbose -eq 1 ]]; then
  echo  "Executing files of ${i: -7}"
  fi
  
  javac "$targets/Java/${i: -7}/Main.java" -d "$targets/Java/${i: -7}/"
  j=1

   for input in $tests/*
    
    do 
   
    java "$targets/Java/${i: -7}/Main.java" < $input  > "$targets/Java/${i: -7}/out$j.txt"
    if  d=$(diff -q "$answers/ans$j.txt"  "$targets/$type/${i: -7}/out$j.txt") ; then
      ((matched++))
    else
      ((notMatched++))
    fi
    ((j++))
    done
  fi
  
   
  fi
  if [[ $noexecute -eq 0 ]]; then
  echo "${i: -7},$type,$matched,$notMatched">>$targets/result.csv
  fi
   
 done
rm -r a