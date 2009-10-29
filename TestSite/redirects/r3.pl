#!/usr/bin/perl

use CGI;                             
$q = new CGI;                        

print $q->redirect("../index.html");
