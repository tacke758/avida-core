#!/usr/bin/perl -w
#
# Make events_auto.cc & events_auto.hh from a simplified template file
# to handle creation of code for simple events
#
# Input file format:
# _event_name_
# args:
# _arg_type _arg_name_ [= _default_value_]
# _arg_type _arg_name_ [= _default_value_]
# ...
# body:
# _body_of_Process()_function_
# ...
#
# _event_name_...

use strict;

$/ = "\n\n";
$" = "|";

my @event_lists = @ARGV; #("cAvidaMain", "cPopulation");

#my $event_dir = "../event/";
my $event_dir = "./";

# Clobber the events_list


my $fname_root = "";
foreach $fname_root (@event_lists){
open(IN,$fname_root."Events_auto") || die $!;


# open the output files
open(LIST,"> ".$event_dir.$fname_root."Events_list");
open(NCI, "> ".$event_dir.$fname_root."Events_MakeEvent_auto.ci");
open(PCI, "> ".$event_dir.$fname_root."Events_auto.ci");
open(DEFS,"> ".$event_dir.$fname_root."Events_DEFS_auto.hh");
open(N2E,"> ".$event_dir.$fname_root."Events_N2E_auto.ci");

print LIST "#### ".$fname_root."Events ####\n\n";

print DEFS "#ifndef ".$fname_root."EVENT_HH\n";
print DEFS "#define ".$fname_root."EVENT_HH\n\n";
print DEFS "class ".$fname_root."Events {\n";
print DEFS "public:\n";
print DEFS "  enum eEvent { EVENT_undefined = 0,\n";

print N2E "int ".$fname_root."EventHandler::EventNameToEnum(const cString & name) const{\n";
print N2E "  if( name == \"\" ){\n";
print N2E "    return ".$fname_root."Events::EVENT_NO_EVENT;\n";


while( <IN> ){
    my @line = split(/\n/);
    my $line_string = "";
    my $i;

    # Filter out blank lines and comments (#)
    for $i (0..$#line){
	$_ = $line[$i];
	s/\#.*$//;
	$line_string .= "$_\n"  unless( /^\s*$/ );
    }
    my ( $name, $tmp_string ) = split(/\:args\:\n/, $line_string);
    my ( $argstring, $body ) = split(/\:body\:\n/, $tmp_string);
    my @args = split(/\n/,$argstring);
    $name =~ /^\s*([^\s]*)\s*$/; $name = $1; # Trim ws off of name

    if( $name ){
	
    ##### Output Class Declaration 
    print PCI "///// $name /////\n";
    print PCI "class ".$fname_root."Event_$name : public ".$fname_root."Event {\nprivate:\n";
    for $i (0..$#args){
	my ($type, $vname, $default) = split(/\s+/, $args[$i]);
	print PCI "  $type $vname;\n";
    }
    $" = ", ";
    print PCI "public:\n";
    print PCI "  ".$fname_root."Event_$name(const cString & in_args):\n";
    print PCI "   ".$fname_root."Event(\"$name\", in_args) {\n";
    
    # Arguemnt intialization
    if( @args ){
	print PCI "\n";
	print PCI "    cString args(in_args);\n";
	for $i (0..$#args){
	    my ($type, $vname, $default) = split(/\s+/, $args[$i]);
# @TCC--- SOMEHOW THIS NEEDS TO TAKE MULTI_WORD DEFAULTS
	    my $default_provided_flag = 0;  # for checking syntax
	    my $pop_cmd = "args.PopWord()";
	    if( $type eq "cString" ){
		# This is the default type 
	    }elsif( $type eq "int" ){
		$pop_cmd .= ".AsInt()";
	    }elsif( $type eq "double" ){
		$pop_cmd .= ".AsDouble()";
	    }
	    if( defined $default ){
		$default_provided_flag = 1;
		print PCI "    (args == \"\") ? $vname=$default : $vname=$pop_cmd;\n";
		#print HI "    $vname = (args == \"\") ? $default : $pop_cmd;\n";
	    }else{
		die "Must Provide Defaults for all variables after first default value\n"  if( $default_provided_flag == 1 );
		print PCI "    $vname = $pop_cmd;\n";
	    }
	}
	print PCI "  }\n";

    }else{
	print PCI " (void)in_args; }\n";
    }

    ##### the process command
    print PCI "///// $name /////\n";
    print PCI "  void Process(){\n";
    my @body_line = split(/\n/,$body);
    for $i (0..$#body_line){
	print PCI "    $body_line[$i]\n";
    }
    print PCI "  }\n";
    print PCI "};\n\n";
    


    ##### CC File stuff... Name to class instansiation
    print NCI "    case ".$fname_root."Events::EVENT_$name :\n";
    print NCI "      event = new ".$fname_root."Event_$name(arg_list);\n";
    print NCI "      break;\n";
    
    ##### Name to Enum #########
    print N2E "  }else if (name == \"$name\") {\n";
    print N2E "    return ".$fname_root."Events::EVENT_$name;\n";

    ##### List of Events #####
    print LIST "$name ";
    for $i (0..$#args){
	my ($type, $vname, $default) = split(/\s+/, $args[$i]);
	if( defined $default ){
	  print LIST " [$type $vname=$default]";
	}else{
	  print LIST " <$type $vname>";
        }
    }
    print LIST "\n"; 

    ###### DEFS ######
    print DEFS "  EVENT_$name,\n";

	
    } # if( $name )
}

print DEFS "EVENT_NO_EVENT };\n";
print DEFS "};\n";
print DEFS "#endif\n";

print N2E "  }else{\n";
print N2E "    return ".$fname_root."Events::EVENT_undefined;\n";
print N2E "  }\n";
print N2E "}\n";

} # for each ARGV to fnameroot

