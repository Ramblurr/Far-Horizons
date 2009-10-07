#!/usr/bin/perl

my $found = 0;
my $show = 0;

while (<>)
{
    if ( /Content-Type:.*text\/plain/ )
    {
	while(<>)
	{
	    chomp;
	    if (length > 0)
	    {
		while(<>)
		{
		    if ( /Content-Type:/ ) 
		    {
			exit 0;
		    }

		    if ( $prev_line )
		    {
			print $prev_line;
		    }
		    $prev_line = $_;
		}
	    }
	}
    }
}
