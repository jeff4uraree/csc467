q: Command not found.
Geoffrey Li

The challenges we came across was how to deal with the shift reduce parsing errors.  The problem was the dangling else problem we were unable to find a solution to fix the dangling else shfit-reduce conflict.  However, since bison outlines that shifts take precedence over reduce, we do not actually have a problem with this aside from a warning message produced by bison.


