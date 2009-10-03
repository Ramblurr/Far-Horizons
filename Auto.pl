#!/bin/perl
#
# The following script will auto-generate galaxy using data piped via stdio
# Player data is expected in the following format(one line per player):
#
# email,sp_name,,home_planet_name,gov_name,gov_type,ML,GV,LS,BI
#
# See example.csv for example
#

# Read input data

my @data;
while (<>)
{
    chomp;
    push @data, [split /,/];
    $num_species++;
}

# TODO: check for duplicate names

print "\n=================== Data provided for $$num_species species. ==================\n";

foreach (@data)
{
    print "@$_\n";
}

print "\n============================= Executing NewGalaxy =============================\n";

# Galaxy size is calculated based on default star density and fixed number of stars per race
# Adjust it here to create less crowded galaxy, players are asking for that
$adjusted_num_species = int(( $num_species * 3) / 2); 

@args = ("NewGalaxy", $adjusted_num_species);
print "@args";
#system(@args) == 0 || die "$args[0] failed: $?"
     
print "\n============================= Executing MakeHomes =============================\n";

@args = ("MakeHomes");
print "@args";
#system(@args) == 0 || die "$args[0] failed: $?"

print "\n============================= Executing ListGalaxy ============================\n";

@args = ("ListGalaxy", "-p");
print "@args";
#system(@args) == 0 || die "$args[0] failed: $?"

# TODO: verify that galaxy contains no duplicate star systems     
     
print "\n============================= Executing MapGalaxy ============================\n";

@args = ("MapGalaxy");
print "@args";
#system(@args) == 0 || die "$args[0] failed: $?"

$curr_sp_number = 1;

print "\n============================= Handling species ============================\n";
foreach (@data)
{
    ( $email, $sp_name, $home_planet, $gov_name, $gov_type, $ML, $GV, $LS, $BI ) = @$_;
    
    print "\n============================= Executing HomeSystem ============================\n";

    $ret = `HomeSystemAuto 12`;
    print "Homesystem returned '$ret'";
    ($x, $y, $z) = split($ret);
    #system(@args) == 0 || die "$args[0] failed: $?"

    # TODO: Parse HomeSystem output & forward to stdout
    
    print "\n=========================== Executing AddSpecies ($curr_sp_number) ============================\n";

    @args = ("AddSpecies", $curr_sp_number, $sp_name, $home_planet, $gov_name, $gov_type, $X, $Y, $Z, $N, $ML, $GV, $LS, $BI );
    print "@args";
    #system(@args) == 0 || die "$args[0] failed: $?"

    print "\n=========================== Adding entry to fh_names ============================\n";

    open FH_NAMES, ">>fh_names" || die ("cannot open fh_names");
    printf FH_NAMES "%02d\n%s\n%s\n", $curr_sp_number, $sp_name, $email;
    
    $curr_sp_number++;
}

print "\n=========================== DONE ============================\n";
