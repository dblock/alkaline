#!/usr/bin/perl

use CGI;                             
$q = new CGI;                        

print $q->redirect("http://www.microsoft.com/");
