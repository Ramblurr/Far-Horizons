#!/usr/bin/perl

use MIME::Parser;

# Create a new MIME parser:
my $parser = MIME::Parser->new;
$parser->output_dir("/tmp");

# Read the MIME message:
$entity = $parser->read(\*STDIN) or die "couldn't parse MIME stream";

$entity->make_multipart; # to allow uniform processing

my @parts = $entity->parts;
my $i;
foreach $i (0 .. $#parts)
{
    # dump each plain text part...
    if ( $parts[$i]->effective_type =~ /text\/plain/ )
    {
	$parts[$i]->print_body;
    }
}


