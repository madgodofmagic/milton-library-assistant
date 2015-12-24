local $/=undef;
open(my $fh, "<", "test.txt");
my $test = <$fh>;
if($test =~ /'''(.*?)'''(.*?)==/sm ) {
    print "title - $1 summary $2";
}
#print $test;
