local $/=undef;
open(my $fh, "<", "satan.wiki");
my $test = <$fh>;
if($test =~ /'''(.*?)'''(.*?)==/sm ) {
    print "title - $1 summary $2";
}
#print $test;
