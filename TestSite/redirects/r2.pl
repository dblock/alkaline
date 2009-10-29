#!/usr/bin/perl

use CGI;                             
$q = new CGI;                        

print $q->redirect("http://www.vestris.com/atest/r2.pl");
