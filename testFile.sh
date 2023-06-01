#! /bin/bash

# read arguments
args=("$@")
len=${#args[@]}
int='^[0-9]+$' # integers
chars=ABCDEFGHIJKLMNOPQRSTUVWXYZ # a string with all alphabetical characters

if [ $len -ne 4 ] # 4 arguments are required
then
  echo "Error: wrong number of args. Use: 4 args"
  exit 1
fi

# save arguments
virusesFile=${args[0]}
countriesFile=${args[1]}
numLines=${args[2]}
duplicatesAllowed=${args[3]}

# check if first two arguments are valid files
if ! [ -e ${args[0]} ] || ! [ -e ${args[1]} ]
then
  echo "Error: given .txt files do not exist"
  exit 1
else
  virusesFile=${args[0]}
  countriesFile=${args[1]}
fi

# check if last two arguments are valid integers
if ! [[ ${args[2]} =~ $int ]] || ! [[ ${args[3]} =~ $int ]]
then
  echo "Error: numLines or duplicatesAllowed val is not an integer"
  exit 1
else
  numLines=${args[2]}
  duplicatesAllowed=${args[3]}
fi

if [[ "$numLines" -gt 10000 || 1 -gt "$numLines" ]]
then
  echo "Error: invalid number of lines. Use: >=1 and <= 10K"
  exit 1
fi

id_array=()
virus_array=()
country_array=()
declare -A hash_array

#read from viruses file, countries file, and save contents to virus_array, country_array
while IFS= read -r virus; do
  virus="${virus::-1}"      # remove newline from virus name, we want to write a single line to output file
  virus_array+=("$virus")
done < "$virusesFile"

while IFS= read -r country; do
  country="${country::-1}"  # remove newline from country name, we want to write a single line to output file
  country_array+=("$country")
done < "$countriesFile" 

# for each line of output file, generate a unique citizen id
id_array=($(shuf --input-range=0-9999 --head-count=${args[2]}))  

# for each line of output file (equivalently for each id of id_array)
for (( i=0; i<${#id_array[@]}; i++))
do
  country=${country_array[$(($RANDOM % ${#country_array[@]}))]}   # select a random country from countries file
  age=$((1 + $RANDOM % 120)) # select a random age in range [1,120]

  namelen=$((3 + $RANDOM % 10)) # select a random name length in range [3,12]
  surlen=$((3 + $RANDOM % 10))  # select a random surname length in range [3,12]
  name=""
  surname=""

  for (( j=0; j<$namelen; j++))
  do
    letter="${chars:RANDOM % ${#chars}:1}"  # select a random char from chars array
    name="${name}$letter"    # append the letter at the end of name
  done  # run for namelen times

  for (( j=0; j<$surlen; j++))
  do
    letter="${chars:RANDOM % ${#chars}:1}"  # select a random char from chars array
    surname="${surname}$letter"    # append the letter at the end of surname
  done  # run for surlen times

  record="$name $surname $country $age"
  hash_array+=(["${id_array[i]}"]="$record")
 
done

if [ $duplicatesAllowed -ne 0 ]
then
  k=$((1+ ($numLines / 100)))
  for (( j=0; j<$k; j++))
  do
    # pick 10 random ids and assign them the same random id (deliberate duplication)
    id=${id_array[$(($RANDOM % ${#id_array[@]}))]}
    #echo $id
    for (( i=0; i<2; i++))
    do
      index=$(($RANDOM % ${#id_array[@]})) # generate a random index
      id_array[$index]=$id
    done
    id=${id_array[$(($RANDOM % ${#id_array[@]}))]}
    #echo $id
    for (( i=0; i<3; i++))
    do
      index=$(($RANDOM % ${#id_array[@]})) # generate a random index
      id_array[$index]=$id
    done
    id=${id_array[$(($RANDOM % ${#id_array[@]}))]}
    #echo $id
    for (( i=0; i<5; i++))
    do
      index=$(($RANDOM % ${#id_array[@]})) # generate a random index
      id_array[$index]=$id
    done
  done
fi

# for each line of output file (equivalently for each id of id_array)
for (( i=0; i<${#id_array[@]}; i++))
do
  virus=${virus_array[$(($RANDOM % ${#virus_array[@]}))]}     # select a random virus from viruses file

  toss=$(($RANDOM % 2)) # give vacc a random number between 0-1 (coin toss to decide if person is vaccinated or not)
  if [ $toss -ne 0 ]
  then
    vacc="YES"                   # person is vaccinated, now set a date
    day=$((1 + $RANDOM % 30))    # randomize a day of a month
    month=$((1 + $RANDOM % 12))  # randomize a month of year
    year=$((2020 + $RANDOM % 3)) # randomize a year from {2020, 2021, 2022}
    date="$day-$month-$year"     # concat to a date
  else
    vacc="NO"   # person is not vaccinated, no date of vaccination is set
  fi

  #write data line to output file
  if [ $toss -ne 0 ]
  then
    # person has been vaccinated at a date
    printf "${id_array[$i]} ${hash_array[${id_array[$i]}]} $virus $vacc $date\n" >> citizenRecordsFile.txt
  else
    # person has not been vaccinated
    printf "${id_array[$i]} ${hash_array[${id_array[$i]}]} $virus $vacc\n" >> citizenRecordsFile.txt   
  fi
done
