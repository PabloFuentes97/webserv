#!/usr/bin/perl

use strict;
use warnings;

my $content_body = $ENV{'CONTENT_BODY'};

my %params;
my @pairs = split(/&/, $content_body);
foreach my $pair (@pairs) {
    my ($key, $value) = split(/=/, $pair);
    $value = '' unless defined $value;
	$value =~ s/\+/ /g;
    $params{$key} = $value;
}


my ($sec, $min, $hour, $day, $month, $year) = localtime();

$year += 1900;
$month += 1;

my $formatted_time = sprintf("%04d-%02d-%02d %02d:%02d:%02d", $year, $month, $day, $hour, $min, $sec);


my $output_file = "SERVER3/cgi_upload/visitlog.txt";
open my $fh, '>>', $output_file or exit (1);

print $fh "\n------\n\n";
foreach my $key (sort keys %params) {
    print $fh "$key: $params{$key}\n";
}
print $fh "TIME: $formatted_time\n";
close $fh;

print "<html><body>Visit logged!\n</body></html>";
