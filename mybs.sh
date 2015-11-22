#!./bin./bash          
echo Executing a bunch of FFMs...

#ffirec tmax sql f A a

# ffirec: A./(1+x./a) form:
 ./ffire5c 500000000 5 250 0 5&
 ./ffire5c 500000000 5 250 .3 5&
 ./ffire5c 500000000 5 250 .6 5&
 ./ffire5c 500000000 5 250 .9 5&
 ./ffire5c 500000000 5 250 1 5&

 ./ffire5c 500000000 5 250 .9 1&
 ./ffire5c 500000000 5 250 .9 5&
 ./ffire5c 500000000 5 250 .9 10&
 ./ffire5c 500000000 5 250 .9 25&
########
# ffire5d: Aexp(-a*(k-1))
 ./ffire5d 500000000 5 250 0 .14&
 ./ffire5d 500000000 5 250 .3 .14&
 ./ffire5d 500000000 5 250 1 .14&

 ./ffire5d 500000000 5 250 .9 .03&
 ./ffire5d 500000000 5 250 .9 .14&
 ./ffire5d 500000000 5 250 .9 .7&
########

# ffire5e: A*k^(-a*k)
 ./ffire5e 500000000 5 250 0 .4&
 ./ffire5e 500000000 5 250 .3 .4&
 ./ffire5e 500000000 5 250 .9 .4&
 ./ffire5e 500000000 5 250 1 .4&

 ./ffire5e 500000000 5 250 .9 .2&
 ./ffire5e 500000000 5 250 .9 .4&
 ./ffire5e 500000000 5 250 .9 .7&
 ./ffire5e 500000000 5 250 .9 1&
