#!/usr/bin/perl
#
# The following script will auto-generate galaxy using data piped via stdio
# Player data is expected in the following format(one line per player):
#
# email,sp_name,,home_planet_name,gov_name,gov_type,ML,GV,LS,BI
#
# See example.csv for example
#
# Execute from game directory: ../Auto.pl < ../example.csv > log.txt
#

my @data;
while (<>)
{
    chomp;
    push @data, [split /\t/];
    $num_species++;
}

# TODO: check for duplicate names

print "\n=================== $num_species species defined. ==================\n";

foreach (@data)
{
    print "@$_\n";
}

print "\n============================= Executing NewGalaxy =============================\n";

# Galaxy size is calculated based on default star density and fixed number of stars per race
# Adjust it here to create less crowded galaxy, players are asking for that
$adjusted_num_species = int(( $num_species * 3) / 2); 

@args = ("../bin/NewGalaxy", $adjusted_num_species);
system(@args) == 0 || die "$args[0] failed: $?";
     
print "\n============================= Executing MakeHomes =============================\n";

@args = ("../bin/MakeHomes");
system(@args) == 0 || die "$args[0] failed: $?";

print "\n============================= Executing ListGalaxy ============================\n";

`../bin/ListGalaxy |sort|uniq -d | wc -l` == 1 || die "Galaxy contains duplicate stars!!!";

$curr_sp_number = 1;

`rm fh_names`;

foreach (@data)
{
    ( $email, $sp_name, $home_planet, $gov_name, $gov_type, $ML, $GV, $LS, $BI ) = split ',', "@$_";
    
    $ret = `../bin/HomeSystemAuto 12`;
    ($x, $y, $z, $n) = split(/ /, $ret);
    
    print "\n=========================== Executing AddSpecies ($curr_sp_number) ============================\n";
    @args = ("../bin/AddSpeciesAuto", $curr_sp_number, $sp_name, $home_planet, $gov_name, $gov_type, $x, $y, $z, $n, $ML, $GV, $LS, $BI );
    system(@args) == 0 || die "$args[0] failed: $?";

    open FH_NAMES, ">>fh_names" || die ("cannot open fh_names");
    printf FH_NAMES "%02d\n%s\n%s\n", $curr_sp_number, $sp_name, $email;
    
    $curr_sp_number++;
}

print "\n=========================== DONE ============================\n";
