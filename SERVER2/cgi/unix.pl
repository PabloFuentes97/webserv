#!/usr/bin/perl

my $query;
    
$query = $ENV{QUERY_STRING};

($field_name, $command) = split (/=/, $query);
if ($command eq "fortune") {
    print "fortune";
} elsif ($command eq "finger") {
    print "finger";
} else {
    print "hello";
 }
exit (0);

