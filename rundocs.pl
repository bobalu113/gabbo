#!/usr/bin/perl

$| = 1;

# FUTURE
# add @see tag support
# add specified by

# TODO
# add inherited methods 
# add real summary support
# add support for some kid of nickname (e.g. RoomCode)

use File::Find;
use File::Path;
use File::Basename;
use Data::Dumper;

$JCPP = "jcpp";
$CYGROOT = "/cygdrive/c/Users/bobal_000/work/gabbo/mudlib";
$ROOT= `cygpath -w $CYGROOT`;
chomp $ROOT;
$ROOT =~ s/\\/\\\\/g;
$DOCS = "/cygdrive/c/Users/bobal_000/work/gabbo-docs";
$TMPFILE = "/tmp/lpcdoc";

@SOURCE = ( "$CYGROOT/lib", "$CYGROOT/modules", "$CYGROOT/secure" );
#@SOURCE = ( "$CYGROOT/lib/strings.c" );
$TYPES = "void|int|string|object|mapping|closure|symbol|float|mixed";
$MODS = "private|protected|static|public|nomask|nosave";
%programs = ();

find(\&generate_doc, @SOURCE);

exit 1;

sub generate_doc {
    return if (/^\./);
    my $cygpath = `cygpath -w $File::Find::name`;
    $cygpath =~ s/\\/\\\\/g;

    my $program = $File::Find::name;
    $program =~ s/^$CYGROOT(.*)\.c$/$1/;

    return if (exists($programs{$program}));

    my $src = "";
    open(F, "$JCPP -I$ROOT\\\\include --root=$ROOT --include=$ROOT\\\\include\\\\auto.h $cygpath |") or die("Couldn't open $TMPFILE for read: $!\n");
    while (<F>) {
        next if (/^\s*$/);
        next if (/^\#/);
        
        s/^\s*(.*?)\s*\n$/$1\n/;
        $src .= $_;
    }
    close(F);

    # if the file begins with a comment, it's the class doc
    my $desc = undef;
    if ($src =~ s#^\s*\/\*\*\s+(.*?)\s+\*\/\s*##s) {
        $desc = &parse_doc($1);
    }

    # grab all the inherit statements before we strip out the strings
    %inherits = ();
    while ($src =~ s/^(.*?inherit.*?);//gm) {
        my $inherit = $1;
        my $prog = "";
        $prog .= $2 while ($inherit =~ /(["'])((\\\1|.)*?)\1/g);
        my $vars = [];
        if ($inherit =~ /\s*(($MODS|\s)+)\s+variables/) {
            $vars = [split(/\s+/, $1)];
        }
        my $funcs = [];
        if ($inherit =~ /\s*(($MODS|\s)+)\s+functions/) {
            $funcs = [split(/\s+/, $1)];
        }
        $inherits{$prog} = [ $vars, $funcs ];
    }

    # first remove all the comments and function bodies
    my $stripped = $src;
    $stripped =~ s#\/\*.*?\*\/##sg;
    @chars = split //, $stripped;
    my $quote = undef;
    my $newsrc = "";
    my $brace = 0;

    for (my $i = 0; $i <= $#chars; $i++) {
        my $char = $chars[$i];
        if (defined($quote)) {
            # if we're in a string literal discard characters
            if ($char eq $quote) {
                my $last_non_bs = $i - 1;
                $last_non_bs-- while($chars[$last_non_bs] eq "\\");
                if (($i - $last_non_bs) % 2) {
                    $quote = undef;
                    next;
                } 
            }
        } else {
            if ($char eq '"') {
                $quote = $char;
            } elsif ($char eq "'") {
                # open quote
                if ($chars[$i-1] eq "#") {
                    next;
                }
                if ($chars[$i+2] eq "'") {
                    $i += 2; 
                }
            } elsif ($char eq "{") {
                # open block
                if (substr($stripped, $i-3, 3) eq "#'(") {
                    next;
                } else {
                    $brace++;
                    next;
                }
            } elsif ($char eq "}") {
                # close block
                $brace--;
                $newsrc .= ";" unless($brace);
            } else {
                # include characters if we're outside all blocks
                if ($brace <= 0) {
                    $newsrc .= $char;
                }
            }
        }
    }

    my %functions = ();
    my %variables = ();
    while ($newsrc =~ m/\s*(.+?)\s*;/gs) {
        my $def = $1;
        $def =~ s/\s*\*\s*/ * /g; # make arrays easier
        if ($def =~ /\s*(.*?)\s*(\S+)\s*\(\s*(.*?)\s*\)\s*/s) {
            # parse out a function definition
            my $prefix = $1;
            my $name = $2;
            my $args = [ ];
            foreach (split /\s*,\s*/, $3) {
                my @s = split /\s+/, $_;
                push @$args, [ "@s[0..($#s-1)]", $s[$#s] ];
            }
            if ($prefix =~ /(.*?)\s*(\w+)\s*(\*?)\s*$/) {
                $mod = $1;
                $type = $2;
                $type .= " *" if($3);
            }
            $functions{$name} = [ $mod, $type, $args, "" ];
        } elsif ($def =~ /\s*(.*?)\s*($TYPES)\s*(.+)\s*/) {
            # parse out a variable definition
            my $mod = $1;
            my $type = $2;
            my %names = ( );
            foreach my $name (split /\s*,\s*/, $3) {
                if ($name =~ /^\s*(.*?)[\s=]*$/) {
                    # get rid of assignments
                    $name = $1;
                }
                if ($name =~ /\s*\*\s*(.*)/) {
                    # it's an array
                    $names{$1} = 1;
                } else {
                    # it's not an array
                    $names{$name} = 0;
                }
            }
            foreach (keys(%names)) {
                $type = "$type *" if ($names{$_});
                $variables{$_} = [ $mod, $type, "" ];
            }
        }
    }

    # go back to unstripped source and look for doc comments
    while ($src =~ m#\/\*\*\s+(.*?)\s+\*\/\s*(.+?)[;{]#sg) {
        my $doc = $1;
        my $code = $2;

        my $tags = &parse_doc($doc);

        if ($code =~ /[\s\*](\w+)\s*\(/s) {
            if (exists($functions{$1})) {
                $functions{$1}->[3] = $tags;
            }
        } elsif ($code =~ /(?:$TYPES)[\s\*]*(\w+)/) {
            if (exists($variables{$1})) {
                $variables{$1}->[2] = $tags;
            }
        }
    }

    my $html = write_doc($program, $desc, \%inherits, \%functions, \%variables);

    my $dir = dirname("$DOCS$program.html");
    mkpath($dir);
    open(F, ">$DOCS/$program.html") or die("Couldn't open $program.html for write: $!\n");
    print F $html;
    close(F);
}

sub parse_doc($) {
    my ( $doc ) = @_;
    $doc =~ s#^\s*\*\s+##gm;
    $doc =~ /(.*?)\s*(\@.*)/ms;
    my $desc = $1;
    $doc = $2;
    my %tags = ();
    my $tag = undef;
    my $arg = undef;
    my $buf = "";
    foreach (split /\n/, $doc) {
        if (/^\@(\w+)\s+(.*)/) {
            if (defined($tag)) {
                if ($tag  eq 'param') {
                    $tags{$tag} = { } unless(exists($tags{$tag}));
                    $tags{$tag}->{$arg} = $buf;
                } else {
                    $tags{$tag} = [ ] unless(exists($tags{$tag}));
                    push @{$tags{$tag}}, [ $arg, $buf ];
                }
                $buf = "";
            }
            $tag = $1;
            $arg = $2;
            if ($tag eq 'param') {
                $arg =~ /^(\w+)\s+(.*)/;
                $arg = $1;
                $buf .= "$2 ";
            } else {
                $buf .= "$arg ";
                $arg = undef;
            }
        } else {
            s/\s*$/ /;
            $buf .= "$_ ";
        }
    }

    if (defined($tag)) {
        if ($tag  eq 'param') {
            $tags{$tag} = { } unless(exists($tags{$tag}));
            $tags{$tag}->{$arg} = $buf;
        } else {
            $tags{$tag} = [ ] unless(exists($tags{$tag}));
            push @{$tags{$tag}}, [ $arg, $buf ];
        }
    }
    return [$desc, \%tags];
}

sub write_doc($$$) {
    my ($program, $desc, $inherits, $functions, $variables) = @_;
    $out = "";
    $out .= <<END;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html lang="en">
<head>
<title>$class</title>
<link rel="stylesheet" type="text/css" href="../stylesheet.css" title="Style">
</head>
<body>
END

    $out .= &navbar("top");

    $out .= <<END;
<!-- ======== START OF CLASS DATA ======== -->
<div class="header">
<!-- <div class="subTitle">$directory</div> -->
<h2 title="Program $program" class="title">Program $program</h2>
</div>
<div class="contentContainer">
<div class="description">
<ul class="blockList">
<li class="blockList">
<dl>
<dt>All Inherited Programs:</dt>
<dd>
<ul>
END
    foreach my $prog (keys(%$inherits)) {
        my $mods = "";
        if (@{$inherits->{$prog}->[0]}) {
            $mods .= join("&nbsp;", @{$inherits->{$prog}->[0]});
            $mods .= " variables";
        }
        if (@{$inherits->{$prog}->[1]}) {
            $mods .= "&nbsp;" if ($mods);
            $mods .= join("&nbsp;", @{$inherits->{$prog}->[1]});
            $mods .= " functions";
        }
        $mods .= "&nbsp;" if ($mods);
        $out .= <<END;
<li>$mods<a href="..$prog.html" title="">$prog</a></li>
END
    }

    $out .= <<END;
</ul>
</dd>
</dl>
<hr>
<br>
<div class="block">$desc->[0]</div>
<!-- 
<dl><dt><span class="strong">See Also:</span></dt><dd>
<a href="../../java/lang/Object.html#toString()"><code>Object.toString()</code></a>
</dd></dl>
-->
</li>
</ul>
</div>
<div class="summary">
<ul class="blockList">
<li class="blockList">
END
    if (%$variables) {
        $out .= <<END;
<!-- =========== FIELD SUMMARY =========== -->
<ul class="blockList">
<li class="blockList"><a name="field_summary">
<!--   -->
</a>
<h3>Variable Summary</h3>
<table class="overviewSummary" border="0" cellpadding="3" cellspacing="0" summary="Variable Summary table, listing fields, and an explanation">
<caption><span>Variables</span><span class="tabEnd">&nbsp;</span></caption>
<tr>
<th class="colFirst" scope="col">Modifier and Type</th>
<th class="colLast" scope="col">Variable and Description</th>
</tr>
END
        my $color;
        foreach my $v (keys(%$variables)) {
            my $var = $variables->{$v};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            my $mod = $var->[0];
            $mod .= "&nbsp;" if ($mod);
            $mod .= $var->[1];
            $out .= <<END;
<tr class="${color}Color">
<td class="colFirst"><code>$mod</code></td>
<td class="colLast"><code><strong><a href="../$program.html#$v">$v</strong></code>
<div class="block"><!-- add summary support --></div>
</td>
</tr>
END
        }
        $out .= <<END;
</table>
</li>
</ul>
END
    }

    if (%$functions) {
        $out .= <<END;
<!-- ========== METHOD SUMMARY =========== -->
<ul class="blockList">
<li class="blockList"><a name="method_summary">
<!--   -->
</a>
<h3>Function Summary</h3>
<table class="overviewSummary" border="0" cellpadding="3" cellspacing="0" summary="Function Summary table, listing functions, and an explanation">
<caption><span>Functions</span><span class="tabEnd">&nbsp;</span></caption>
<tr>
<th class="colFirst" scope="col">Modifier and Type</th>
<th class="colLast" scope="col">Function and Description</th>
</tr>
END

        foreach my $f (keys(%$functions)) {
            my $func = $functions->{$f};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            $args = "";
            my $first = 1;
            for (@{$func->[2]}) {
                $args .= ", " unless($first);
                $args .= "$_->[0]";
                $args .= "&nbsp;" unless ($_->[0] =~ /\*$/); 
                $args .= $_->[1];
                $first = 0;
            }
            my $mod = $func->[0];
            $mod .= "&nbsp;" if ($mod);
            $mod .= $func->[1];
            $out .= <<END;
<tr class="${color}Color">
<td class="colFirst"><code>$mod</code></td>
<td class="colLast"><code><strong><a href="../..$program.html#$f()">$f</a></strong>($args)</code>
<div class="block"><!-- implement summaries --></div>
</td>
</tr>
END
        }
        $out .= <<END;
</table>
<!--
<ul class="blockList">
<li class="blockList"><a name="methods_inherited_from_class_java.lang.Object">
</a>
<h3>Methods inherited from class&nbsp;java.lang.<a href="../../java/lang/Object.html" title="class in java.lang">Object</a></h3>
<code><a href="../../java/lang/Object.html#clone()">clone</a>, <a href="../../java/lang/Object.html#finalize()">finalize</a>, <a href="../../java/lang/Object.html#getClass()">getClass</a>, <a href="../../java/lang/Object.html#notify()">notify</a>, <a href="../../java/lang/Object.html#notifyAll()">notifyAll</a>, <a href="../../java/lang/Object.html#wait()">wait</a>, <a href="../../java/lang/Object.html#wait(long)">wait</a>, <a href="../../java/lang/Object.html#wait(long,%20int)">wait</a></code></li>
</ul>
-->
END
    }
    $out .= <<END;
</li>
</ul>
</li>
</ul>
</div>
<div class="details">
<ul class="blockList">
<li class="blockList">
END

    if (%$variables) {
        $out .= <<END;
<!-- ============ FIELD DETAIL =========== -->
<ul class="blockList">
<li class="blockList"><a name="field_detail">
<!--   -->
</a>
<h3>Variable Detail</h3>
END
        foreach my $v (keys(%$variables)) {
            my $var = $variables->{$v};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            my $mod = $var->[0];
            $mod .= "&nbsp;" if ($mod);
            $mod .= $var->[1];
            $mod .= "&nbsp;" if ($mod);
            $out .= <<END;
<a name="$v">
<!--   -->
</a>
<ul class="blockListLast">
<li class="blockList">
<h4>$v</h4>
<pre>$mod$v</pre>
<div class="block">$var->[2]->[0]</div>
<!--
<dl><dt><span class="strong">Since:</span></dt>
  <dd>1.2</dd>
<dt><span class="strong">See Also:</span></dt><dd><a href="../../java/text/Collator.html#compare(java.lang.String,%20java.lang.String)"><code>Collator.compare(String, String)</code></a></dd></dl>
-->
</li>
</ul>
END
        }

        $out .= <<END;
</li>
</ul>
END
    }

    if (%$functions) {
        $out .= <<END;
<!-- ============ METHOD DETAIL ========== -->
<ul class="blockList">
<li class="blockList"><a name="method_detail">
<!--   -->
</a>
<h3>Function Detail</h3>
END

        foreach my $f (keys(%$functions)) {
            my $func = $functions->{$f};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            $args = "";
            my $first = 1;
            for (@{$func->[2]}) {
                $args .= ", " unless($first);
                $args .= "$_->[0]";
                $args .= "&nbsp;" unless ($_->[0] =~ /\*$/); 
                $args .= $_->[1];
                $first = 0;
            }
            my $mod = $var->[0];
            $mod .= "&nbsp;" if ($mod);
            $mod .= $var->[1];
            $mod .= "&nbsp;" if ($mod);
            $out .= <<END;
<a name="$f()">
<!--   -->
</a>
<ul class="blockList">
<li class="blockList">
<h4>$f</h4>
<pre>$mod$f($args)</pre>
<div class="block">$func->[3]->[0]</div>
<dl>
<!--
<dt><strong>Specified by:</strong></dt>
<dd><code><a href="../../java/lang/CharSequence.html#length()">length</a></code>&nbsp;in interface&nbsp;<code><a href="../../java/lang/CharSequence.html" title="interface in java.lang">CharSequence</a></code></dd>
-->
<dl>
END
            if (@{$func->[2]}) {
                $out .= <<END;
<dt><span class="strong">Parameters:</span></dt>
END
                for (@{$func->[2]}) {
                    $out .= <<END;                    
<dd><code>$_->[1]</code> - $func->[3]->[1]->{'param'}->{$_->[1]}</dd>
END
                 }
            }

            if ($func->[3]->[1]->{'return'}->[0]->[1]) {
                $out .= <<END;
<dt><span class="strong">Returns:</span></dt><dd>$func->[3]->[1]->{'return'}->[0]->[1]</dd>
END
            }

            $out .= <<END;
</dl>
</li>
</ul>
END

        }
        $out .= <<END;
</li>
</ul>
</li>
</ul>
END
    }

    $out .= <<END;
</li>
</ul>
</li>
</ul>
</div>
</div>
<!-- ======== END OF CLASS DATA ======== -->
END

    $out .= navbar("bottom");

    $out .= <<END;
</body>
</html>
END

    return $out;
}

sub navbar($) {
    my ($loc) = @_;
    return <<END;
<div class="${loc}Nav"><a name="navbar_$loc">
</a><a href="#skip-navbar_$loc" title="Skip navigation links"></a><a name="navbar_$loc_firstrow">
</a>
<ul class="navList" title="Navigation">
<!--
<div class="aboutLanguage"><em><strong>LDMud Foundational Mudlib<br/>version 0.1</strong></em></div>
-->
</div>
<div class="subNav">
<ul class="navList">
<li><a href="" title="class in java.lang"><span class="strong">Prev Program</span></a></li>
<li><a href="" title="class in java.lang"><span class="strong">Next Program</span></a></li>
</ul>
<ul class="navList">
<li><a href="../../index.html?java/lang/String.html" target="_top">Frames</a></li>
<li><a href="$class.html" target="_top">No Frames</a></li>
</ul>
<ul class="navList" id="allclasses_navbar_top">
<li><a href="../../allclasses-noframe.html">All Program</a></li>
</ul>
<div>
<script type="text/javascript"><!--
  allClassesLink = document.getElementById("allclasses_navbar_top");
  if(window==top) {
    allClassesLink.style.display = "block";
  }
  else {
    allClassesLink.style.display = "none";
  }
  //-->
</script>
</div>
<div>
<ul class="subNavList">
<li>Summary:&nbsp;</li>
<li><a href="#field_summary">Variable</a>&nbsp;|&nbsp;</li>
<li><a href="#method_summary">Function</a></li>
</ul>
<ul class="subNavList">
<li>Detail:&nbsp;</li>
<li><a href="#field_detail">Variable</a>&nbsp;|&nbsp;</li>
<li><a href="#method_detail">Function</a></li>
</ul>
</div>
<a name="skip-navbar_$loc">
<!--   -->
</a></div>
END
}