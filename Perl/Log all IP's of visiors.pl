#!/usr/bin/perl
# RapMaster2000 <RapMaster2000@mail.ru>
# This scripts log all IP's of visiors who came to ur site using proxy 
# Setup: use SSI (if dont what hell is it check out www.apache.org's faq
# or use image example: <img src="http://www.mysite.com/cgi-bin/proxylog.pl">
use Socket;
$pic_url = "http://www.aol.com/gr/homepage/logo.gif";
if(!$ENV{'REMOTE_ADDR'}) { print "Not http request\n"; exit(0);}
my($rhost,$i,$paddr,$sux);
$rhost = $ENV{'REMOTE_ADDR'};
@ports = (1080,3128,8080);
open(DB,">>log.txt");
for($i=0;$i<=2;$i++) {
socket(SOCKET, PF_INET, SOCK_STREAM, getprotobyname('tcp'));
$paddr  = sockaddr_in($ports[$i],inet_aton($rhost));
if (connect(SOCKET, $paddr)) {
open(DB1,"log.txt");
while(<DB1>){
($addr,$port) = split(':',$_);
if($rhost eq $addr){
$sux = 1;}
}
close DB1;
if($sux !=1){
print DB "$rhost:$ports[$i]\n";
}
close (SOCKET);
}
else {
close (SOCKET);
}}
close DB;
print "Location: $pic_url \n\n";
