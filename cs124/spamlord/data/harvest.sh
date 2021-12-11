for i in $(ls dev/); do
    grep -P "[0-9]{3}[^0-9]{1,2}[0-9]{3}[^0-9]{1,2}[0-9]{4}" < dev/$i
done;
