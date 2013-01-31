#!/usr/bin/perl
# phpBB version check.
# Coder: Saime
# Usage: perl phpbb.pl http://site.com/forum/
# http://saime.biz
use LWP::UserAgent;
$site = $ARGV[0];
$brows = LWP::UserAgent->new;
$res = $brows->get($site.'/docs/CHANGELOG.html');
if($res->is_success)
{
@ver=$res->content=~/<li><a href="(.*?)\">Changes since (.*?)<\/a><\/li>/i;
$ver[1]=~/(\d+)\.(\d+)\.(\d+)/;
$version=$1.'.'.$2.'.'.scalar $3+1;
print 'Version: '.$version if $version; }