# Perl Threads Support #

With the help of Perl threads support, you can run more than one tasks simultaneously in SeqPipe pipeline. There are two options in Perl to support threads:
  * ithread - this may require you to re-build Perl.
  * forks - this is a perl package.

Therefore, 'forks' is the best choice. You can install it via cpan:
```
$ cpan
cpan[1]> install forks
```
Say "yes" when it asks "`Would you like to create references to forks, such that using 'use threads' and 'use threads::shared' will quietly load forks and forks::shared? [no]`".