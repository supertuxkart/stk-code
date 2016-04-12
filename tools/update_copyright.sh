#!/usr/bin/awk -f
# A simple awk script to update copyright lines
# It can be used in a simple loop, e.g.:
#
# cd src
# for i in $a; do 
#     echo $i
#     ../update_copyright.sh $i >xx
#     rc=$?
#     if [ $rc == "0" ]; then 
#         echo "$i contains no (c)."
#     else
#         mv xx $i
#     fi
# done


BEGIN   {
            found_something=0; 
        }
/\(C\)/ {
            if(index($4,"-")>0) 
               new_years=gensub("-.*$","-2015",1,$4); 
	    else
               new_years=$4"-2015";
	    line = $0;
            sub($4,new_years,line);
	    print line;
	    found_something=1;
  	    next;
        }
        { 
	    print $0;
        }
END     {
            exit(found_something);
        }

