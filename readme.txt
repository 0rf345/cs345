
	csd3285
	Orfeas Kalipolitis

It works as advertised:
make all -> creates cs345sh
make clean -> destroys cs345sh (nothing else is not needed)
for daemons just add '&' to the end of the process name (no arguments)
set var="command arg1 arg2 ... argN" cannot use specials (|,>,<,>>)
unset var
You can either use '|', '<', '>' or '&', not all at the same line
 
