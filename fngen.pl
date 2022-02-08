#!/usr/bin/perl
#
use strict;

my %FTAB = ();


while (<STDIN>) {
  if (/^\s*int\s+([_a-zA-Z0-9]+)\(.*\)\s+\/\*\s+DESC:\s+(.+)\s+\*\/\s*/) {
    my ($fname,$fdesc) = ($1,$2);
    my $fid = $fname;
    $fid =~ s/^mpf_//;
    $fid =~ tr/_/-/;
    
    $FTAB{$fid} = { 'ID' => $fid, 'DESC' => $fdesc, 'NAME' => $fname };

  }
}

my @FLST = ();

foreach my $k (sort keys %FTAB) {
  push @FLST,$FTAB{$k}
}


#
# Binary tree
#
my $r = undef;

sub addnode {
  my ($r,$n) = @_;

  if ($r) {
    if ($r->{ID} lt $n->{ID}) {
      $r->{LEFT} = addnode($r->{LEFT},$n);
    } elsif ($r->{ID} gt $n->{ID}) {
      $r->{RIGHT} = addnode($r->{RIGHT},$n);
    } else {
      die "DUPLICATE FUNCTION: $n->{ID}";
    }
    return $r;  
  } else {
    return $n;
  }
}


sub showref {
  my ($n) = @_;

  if ($n) {
    return "_node_$n->{NAME}";
  } else {
    return "NULL";
  }
}

sub outnode {
  my ($r) = @_;

  print "struct _mpf_functions _node_$r->{NAME} [] = { {\n";
  print "  ",showref($r->{LEFT}),",",showref($r->{RIGHT}),",\n";
  print "  \"$r->{ID}\",$r->{NAME},LL(\"$r->{DESC}\"),1\n";
  print "} };\n";
}


sub dumptree {
  my ($r) = @_;

  if ($r) {
    dumptree($r->{LEFT});
    dumptree($r->{RIGHT});
    outnode($r); 
  }
}


#
# Navigate list...
#
sub navlist {
  my ($l,$s,$c) = @_;

  return unless ($c);

  my $m = int($c/2);

  $r = addnode($r,$l->[$s+$m]);
  navlist($l,$s,$m);
  ++$m;
  navlist($l,$s+$m, $c-$m);
}


navlist(\@FLST,0,scalar(@FLST));

dumptree($r);

print "struct _mpf_functions *mpf_functions = ",showref($r),";\n";
