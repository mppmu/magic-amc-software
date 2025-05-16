#!/usr/bin/perl
#
# $Id: epccontrol2.pl,v 2.2 2007/05/03 08:46:14 martinb1 Exp $
#
###############################################################################
#                                                                             #
# CommandLine Tool to access the ExpertPowerControl (Rev2)                    #
#                                                                             #
# sample calls:                                                               #
#                                                                             #
#    Show current State:                                                      #
#       - epccontrol2.pl --host=192.168.0.2 --password=xyz                    #
#                                                                             #
#    Switch On:                                                               #
#       - epccontrol2.pl --host=192.168.0.2 --password=xyz -on                #
#                                                                             #
#    Switch Off:                                                              #
#       - epccontrol2.pl --host=192.168.0.2 --password=xyz -off               #
#                                                                             #
#    Toggle:                                                                  #
#       - epccontrol2.pl --host=192.168.0.2 --password=xyz -t                 #
#                                                                             #
###############################################################################
#                                                                             #
# (c)2004, Author: Martin Bachem, Gude Analog- und Digitalsysteme GmbH        #
#       http://www.gude.info/                                                 #
#                                                                             #
###############################################################################
#

$adminname = "admin";
$username  = "user";
$password = "password";
$host = "161.72.130.96";

###############################################################################
# Command Line Parameters Defaults
#
$powerswitch=1;
$action=0;         # 1=on, 2=off, 3=toggle

###############################################################################
# gloval vars
$num_ports=0;
$switch_all=0;


$delay=2; # 2 seconds delay when switching all ports
$batch=0; # idle T
$batch_t = 0; # default: time in seconds


sub usage {
        print ("epccontrol2.pl\n");
        print ("  [--host=192.168.0.2]       : define the IP-Adress of your device)\n");
        print ("  [--password=mypasswd]      : define HTTP Password if needed\n");
        print ("  [--port=X],[-p=X]          : select switch port no. X (default: 1)\n");
        print ("  [--switch_on], [-on]       : switch PowerSwitch X ON\n");
        print ("  [--switch_off],[-off]      : switch PowerSwitch X OFF\n");
        print ("  [--switch_toggle],[-t]     : toggle PowerSwitch X from ON<->OFF\n");
        print ("  [--batch_onoff=T],[-bon=T] : switch port on, after T seconds switch off");
        print ("  [--batch_offon=T],[-bof=T] : switch port off, after T seconds switch on");
        print ("                                  T: 1=1s,  2=2s,  3=3s,  4=4s, 5=5s,");
        print ("                                     6=10s, 7=15s, 8=20s, 9=30s");
        print ("  [--batch_min], [-bm]       : handle values for T (see above) as minute values");
        print ("  [--all,[-a]                : switch all available ports\n");
        print ("  [--delay=X,[-d=X]          : when --all, delay X seconds after\n");
        print ("                               each port (default: 2s)\n");
        print ("  [--silent],[-s]            : no console output\n");
        exit;
}


foreach $cmd_line_param (@ARGV) {

        # --host=192.168.0.2
        if ($cmd_line_param =~ "--host=") {
                @param_vals = split(/=/, $cmd_line_param);
                if ($param_vals[1] ne "") {
                        $host = $param_vals[1];
                }
        }

        # --password=mypasswd
        if ($cmd_line_param =~ "--password=") {
                @param_vals = split(/=/, $cmd_line_param);
                if ($param_vals[1] ne "") {
                        $password = $param_vals[1];
                }
        }

        # [--powerswitch=X], [-p=X]
        if (($cmd_line_param =~ "--powerswitch=")  || ($cmd_line_param =~ "-p=")) {
                @param_vals = split(/=/, $cmd_line_param);
                if ($param_vals[1] ne "") {
                        $powerswitch = $param_vals[1];
                }
        }

        # [--switch_on], [-on]  --> action = 1;
        if (($cmd_line_param eq "--switch_on") || ($cmd_line_param eq "-on")) {
                $action = 1;            
        }

        # [--switch_off], [-off]  --> action = 2;
        if (($cmd_line_param eq "--switch_off") || ($cmd_line_param eq "-off")) {
                $action = 2;            
        }

        # [--switch_toggle], [-t]  --> action = 3;
        if (($cmd_line_param eq "--switch_toggle") || ($cmd_line_param eq "-t")) {
                $action = 3;            
        }

        # [--all] : do action for all ports (default delay is 2 seconds, modify with --delay)
        if (($cmd_line_param eq "--all") || ($cmd_line_param eq "-a")) {
                $switch_all = 1;
        }

        # [--batch_onoff=T], [-bon=X] : reset port (switch on, wait, switch off)
        if (($cmd_line_param =~ "--batch_onoff=")  || ($cmd_line_param =~ "-bon=")) {
                @param_vals = split(/=/, $cmd_line_param);
                if ($param_vals[1] ne "") {
                        $action = 4;
                        $batch = $param_vals[1];
                }
        }
        
        # [--batch_onoff=T], [-bon=X] : reset port (switch on, wait, switch off)
        if (($cmd_line_param =~ "--batch_offon=")  || ($cmd_line_param =~ "-bof=")) {
                @param_vals = split(/=/, $cmd_line_param);
                if ($param_vals[1] ne "") {
                        $action = 5;
                        $batch = $param_vals[1];
                }
        }
        

        # [--batch_min], [-bm] : no console output
        if (($cmd_line_param eq "--batch_min") || ($cmd_line_param eq "-bm")) {
                $batch_t = 1;
        }       

        # [--delay=X], [-d=X] : delay in seconds for allon / alloff / alltoggle
        if (($cmd_line_param =~ "--delay=")  || ($cmd_line_param =~ "-d=")) {
                @param_vals = split(/=/, $cmd_line_param);
                if ($param_vals[1] ne "") {
                        $delay = $param_vals[1];
                }
        }

        # [--silent], [-s] : no console output
        if (($cmd_line_param eq "--silent") || ($cmd_line_param eq "-s")) {
                $silent_mode = 1;
        }

        # [--help], [-h]  --> print usage;
        if (($cmd_line_param eq "--help") || ($cmd_line_param eq "-h")) {
                usage();
        }       

        # [-on=X]
        if ($cmd_line_param =~ "-on=")  {
                @param_vals = split(/=/, $cmd_line_param);
                if (($param_vals[1] > 0) && ($param_vals[1] < 9)) {
                        $powerswitch = $param_vals[1];
                        $action = 1 ;
                }
        }

        # [-off=X]
        if ($cmd_line_param =~ "-off=")  {
                @param_vals = split(/=/, $cmd_line_param);
                if (($param_vals[1] > 0) && ($param_vals[1] < 9)) {
                        $powerswitch = $param_vals[1];
                        $action = 2 ;
                }
        }





}

###############################################################################
# Include Perl HTTP module
#
eval("use LWP::UserAgent");
if ($@) {
        print "ERROR: Module LWP::UserAgent not on found this host\n";
        exit;
}

if (!($host =~ ":"))  {
        $host = $host.':80';
}
$full_url = "http://".$host."/";
$state_txt{"1"} = "ON";
$state_txt{"0"} = "OFF";

###############################################################################
# Support Sub-Routines
#
sub print_current_state {
        if ($silent_mode == 0) {
                for ($i=0; $i<$num_ports; $i++) {
                        print ($port_names[$i]);
                        print (" is ");
                        print ($state_txt{$port_states[$i]});   
                        print ("\n");
                }
        }
}

sub get_mode {
        $response = $browser->get($full_url);
        @lines = split(/\n/, $response->content);
        
        $num_ports = 0;
        $all_stats = 0;
        
        foreach $line (@lines) {
                if ($line =~ "powerstate") {
                        @params = split(/\"/, $line);
                        if ($params[3] =~ ",") {
                                
                                @current_state = split(/,/, $params[3]);
                                $port_names[$num_ports] = $current_state[0];
                                $port_states[$num_ports] = $current_state[1];
                                
                                $all_stats = $all_stats * 2 ;
                                if ( $current_state[1] > 0 ) { $all_stats = $all_stats+1; }

                                $num_ports = $num_ports+1;
                        }
                }
        }
        
        return ($num_ports > 0);
}

sub login {
        my $login_ok = 0;

        # just login 
        if ($action == 0) {
                $response = $browser->get($full_url);
        } else {
                if ($num_ports == 1) {
                        $response = $browser->get($full_url."switch.html");
                } else {
                        $response = $browser->get($full_url."ov.html");
                }
        }
        
        # Login successfull...  
        if ($response->is_success) {
                $login_ok = 1;
                @lines = split(/\n/, $response->content);
                foreach $line (@lines) {
                        if ($line =~ "blocked") {
                                if ($silent_mode == 0) {
                                        print ("ACCESS DENIED! $line\n");
                                }
                                $login_ok = 0;
                        }
                }
        }
        
        return ($login_ok);
}


sub switch_action() {
        # Toggle ?
        if ($action == 3) { 
                if ($port_states[$powerswitch - 1] == 0) {
                        $action = 1;
                } else {
                        $action = 2;
                }
        }

        # Switch ON ?           
        if ($action == 1) { 
                $http_get = $full_url."switch.html?cmd=1&p=".$powerswitch."&s=1";
        }

        # Switch OFF ?
        if ($action == 2) { 
                $http_get = $full_url."switch.html?cmd=1&p=".$powerswitch."&s=0";
        }

        # batchmode on->wait->off
        if ($action == 4) { 
                $http_get = $full_url."batch.html?cmd=5&p=".$powerswitch."&a1=1&w=".$batch."&t=".$batch_t."&a2=0";
        }
                
        # batchmode off->wait->on
        if ($action == 5) { 
                $http_get = $full_url."batch.html?cmd=5&p=".$powerswitch."&a1=0&w=".$batch."&t=".$batch_t."&a2=1";
        }
        
        if ($silent_mode == 0) {
                print ("\nsending HTTP GET: '$http_get'\n\n");
        }
        $response = $browser->get($http_get);

}

################################################################################################
# MAIN APP
#


if ($host eq "") {
        print ("\nparameter --host is missing! try 'epccontrol2.pl --host=192.168.x.x'\n");
        print ("or 'epccontrol2.pl --help\n\n");
        exit;
}

$browser = LWP::UserAgent->new;
$browser->credentials(
    $host,
    'Enter Password',
    $adminname => $password
);
$browser->agent($ENV{SCRIPT_NAME});  

if (get_mode()) {
        if (login()) {

                if ($switch_all) {
                        if ($delay < 1) {
                                print ("ERROR: delay musst be >= 1s");
                                exit;
                        }
                        
                        for ($i=0; $i<$num_ports; $i++) {
                                $powerswitch = $i+1;
                                $save_action = $action;
                                switch_action();
                                $action = $save_action;
                                sleep($delay);
                        }
                        print ("\n");
                                                
                } else {
                        switch_action();
                }

                get_mode();
                print_current_state;
                
                # do Logout
                $response = $browser->get($full_url);
                
        } else {
                print "ERROR: " . $response->code. " " . $response->message;    
                if ($response->code eq 401) {
                        print ("\ntry \"epccontrol2.pl --password=YourPassword\"\n");
                }
        }
}       else {
        printf ("Could not connect $full_url !\n");
}
exit $all_stats ;

