#!/usr/bin/perl

$| = 1;

# FUTURE
# add support for: @see, @since, @link, @deprecated, @inheritDoc
# add support for some kid of nickname (e.g. RoomCode)
# fix the ugly

# TODO
# add full inheritance tree
# add inherited members

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
#@SOURCE = ( "$CYGROOT/lib/doctest.c" );

%MODRANKS = ( "public" => 1,
              "static" => 2,
              "protected" => 3,
              "private" => 4,
              "nomask" => 5,
              "nosave" => 6,
              "varargs" => 7,
              "virtual" => 8 );
$MODS = join "|", keys(%MODRANKS);
$TYPES = "void|int|string|object|mapping|closure|symbol|float|mixed";

find(\&generate_doc, @SOURCE);

exit 1;

sub generate_doc {
    return if (/^\./);
    my $cygpath = `cygpath -w $File::Find::name`;
    $cygpath =~ s/\\/\\\\/g;

    my $program = $File::Find::name;
    $program =~ s/^$CYGROOT(.*)\.c$/$1/;

    # run the file through the preprocessor
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
    my $i = 1;
    while ($src =~ s/^(.*?inherit.*?);//gm) {
        my $inherit = $1;
        my $prog = "";
        # FIXME resolve relative paths to full program name
        $prog .= $2 while ($inherit =~ /(["'])((\\\1|.)*?)\1/g);
        # check for variable modifiers
        my $vars = { };
        if ($inherit =~ /\s*((?:$MODS|\s)+)\s+variables/) {
            my $rank = 1;
            $vars = { map { $_ => $rank++ } split(/\s+/, $1) };
        }
        # check for function modifiers
        my $funcs = { };
        if ($inherit =~ /\s*((?:$MODS|\s)+)\s+functions/) {
            my $rank = 1;
            $funcs = { map { $_ => $rank++ } split(/\s+/, $1) };
        }
        # check for virtual mod
        my $mods = { };
        if ($inherit =~ /\s*((?:$MODS|\s)+)\s+inherit/) {
            my $rank = 1;
            $mods = { map { $_ => $rank++ } split(/\s+/, $1) };
        }
        $inherits{$prog} = [ $vars, $funcs, $mods, $i++ ];
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
                    # closure
                    next;
                }
                if ($chars[$i+2] eq "'") {
                    #quoted character, skip
                    $i += 2; 
                }
            } elsif ($char eq "{") {
                if (substr($stripped, $i-3, 3) eq "#'(") {
                    # array constructor closure
                    next;
                } else {
                    # open block, descend and start/continue stripping
                    $brace++;
                    next;
                }
            } elsif ($char eq "}") {
                # close block, back out and convert to function prototype
                $brace--;
                $newsrc .= ";" unless($brace);
            } else {
                if ($brace <= 0) {
                    # include characters if we're outside all blocks
                    $newsrc .= $char;
                }
            }
        }
    }

    my %functions = ();
    my %variables = ();
    my $i = 0;
    while ($newsrc =~ m/\s*(.+?)\s*;/gs) {
        my $def = $1;
        $def =~ s/\s*\*\s*/ * /g; # make arrays easier
        if ($def =~ /\s*(.*?)\s*(\S+)\s*\(\s*(.*?)\s*\)\s*/s) {
            # parse out a function definition
            my $prefix = $1;
            my $name = $2;
            my $args = [ ];
            # parse out the args
            foreach (split /\s*,\s*/, $3) {
                my @s = split /\s+/, $_;
                push @$args, [ "@s[0..($#s-1)]", $s[$#s] ];
            }
            # now the type and modifiers
            if ($prefix =~ /\s*((?:$MODS|\s)*)\s*($TYPES)\s*(\*?)\s*/) {
                my $rank = 1;
                $mods = { map { $_ => $rank++ } split(/\s+/, $1) };
                $type = $2;
                $type .= " $3";
            }
            $functions{$name} = [ $mods, $type, $args, "", $i++ ];
        } elsif ($def =~ /\s*((?:$MODS|\s)*)\s*($TYPES)\s*(.+)\s*/) {
            # parse out a variable definition
            my $rank = 1;
            $mods = { map { $_ => $rank++ } split(/\s+/, $1) };
            my $type = $2;
            # handle multiple variable declarations on the same line
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
                $variables{$_} = [ $mods, $type, "", $i++ ];
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
    my $rel = substr("/.." x (scalar(split(/\/+/, $program)) - 2), 1);
    $out = "";
    $out .= <<END;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html lang="en">
<head>
<title>$class</title>
<link rel="stylesheet" type="text/css" href="$rel/stylesheet.css" title="Style">
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
END
    if (%$inherits) {
        $out .= <<END;
<dl>
<dt>All Inherited Programs:</dt>
<dd>
<ul>
END
        my @inh = sort {
            $inherits->{$a}->[3] <=> $inherits->{$b}->[3]
        } keys(%$inherits);
        foreach my $prog (@inh) {
            my $mods = &inherit_mods($inherits->{$prog});
            $mods = "&nbsp;[$mods]" if ($mods);
            $out .= <<END;
<li><a href="$rel$prog.html" title="">$prog</a>$mods</li>
END
        }
        $out .= <<END;
</ul>
</dd>
</dl>
END
    }

    $out .= <<END;
<hr>
<br>
<div class="block">$desc->[0]</div>
<!-- 
<dl><dt><span class="strong">See Also:</span></dt><dd>
<a href="$rel$prg.html#toString()"><code>$prg->toString()</code></a>
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
        my @vars = sort keys(%$variables);
        foreach my $v (@vars) {
            my $var = $variables->{$v};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            my $mods = &variable_mods($var);
            $mods .= "&nbsp;" if ($mods);
            my $type = $var->[1];
            $type =~ s/\s+/&nbsp;/g;
            my $summary = $var->[2]->[0];
            $summary =~ s/^(.*?\.)\s.*/$1/s;
            $out .= <<END;
<tr class="${color}Color">
<td class="colFirst"><code>$mods$type</code></td>
<td class="colLast"><code><strong><a href="$rel/$program.html#$v">$v</strong></code>
<div class="block">$summary</div>
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

        my @funcs = sort keys(%$functions);
        foreach my $f (@funcs) {
            my $func = $functions->{$f};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            my $mods = &function_mods($func);
            $mods .= "&nbsp;" if ($mods);
            my $type = $func->[1];
            $type =~ s/\s+/&nbsp;/g;
            my $args = &function_args($func);
            my $summary = $func->[3]->[0];
            $summary =~ s/^(.*?\.)\s.*/$1/s;
            $out .= <<END;
<tr class="${color}Color">
<td class="colFirst"><code>$mod$type</code></td>
<td class="colLast"><code><strong><a href="$rel$program.html#$f()">$f</a></strong>($args)</code>
<div class="block">$summary</div>
</td>
</tr>
END
        }
        $out .= <<END;
</table>
<!--
<ul class="blockList">
<li class="blockList"><a name="methods_inherited_from_class_$prg">
</a>
<h3>Functions inherited from program&nbsp;<a href="$rel$prg.html" title="class in java.lang">$prg</a></h3>
<code><a href="$rel$prg.html#clone()">clone</a>, <a href="$rel$prg.html#wait(long,%20int)">wait</a></code></li>
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
        my @vars = sort { 
            $variables->{$a}->[3] <=> $variables->{$b}->[3] 
        } keys(%{$variables});
        foreach my $v (@vars) {
            my $var = $variables->{$v};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            my $mods = &variable_mods($var);
            $mods .= "&nbsp;" if ($mods);
            my $type = $var->[1];
            $type =~ s/\s+/&nbsp;/g;
            $type .= "&nbsp;" if ($type);
            $out .= <<END;
<a name="$v">
<!--   -->
</a>
<ul class="blockListLast">
<li class="blockList">
<h4>$v</h4>
<pre>$mods$type$v</pre>
<div class="block">$var->[2]->[0]</div>
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

        my @funcs = sort { 
            $functions->{$a}->[4] <=> $functions->{$b}->[4] 
        } keys(%{$functions});
        foreach my $f (@funcs) {
            my $func = $functions->{$f};
            if ($color eq "row") { $color = "alt"; }
            else { $color = "row"; }
            my $args = &function_args($func);
            my $mods = &function_mods($func);
            $mods .= "&nbsp;" if ($mods);
            my $type = $func->[1];
            $type =~ s/\s+/&nbsp;/g;
            $out .= <<END;
<a name="$f()">
<!--   -->
</a>
<ul class="blockList">
<li class="blockList">
<h4>$f</h4>
<pre>$mod$type$f($args)</pre>
<div class="block">$func->[3]->[0]</div>
<dl>
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
<div class="aboutLanguage"><em><strong>GABBO Foundation<br/>version 0.1</strong></em></div>
-->
</div>
<div class="subNav">
<ul class="navList">
<li><a href="" title="class in java.lang"><span class="strong">Prev Program</span></a></li>
<li><a href="" title="class in java.lang"><span class="strong">Next Program</span></a></li>
</ul>
<ul class="navList">
<li><a href="../../index.html?$program.html" target="_top">Frames</a></li>
<li><a href="$rel$program.html" target="_top">No Frames</a></li>
</ul>
<ul class="navList" id="allclasses_navbar_top">
<li><a href="$rel/allclasses-noframe.html">All Programs</a></li>
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

sub inherit_mods($) {
    my ($inherit, $preserve_order) = @_;
    my %ranks;
    my $mods = "";
    my %v = %{ $inherit->[0] };
    %ranks = ( $preserve_order ? %v : %MODRANKS );
    if (%v) {
        $mods .= join("&nbsp;", sort { $ranks{$a} <=> $ranks{$b} } keys(%v));
        $mods .= "&nbsp;variables";
    }
    my %f = %{ $inherit->[1] };
    %ranks = ( $preserve_order ? %f : %MODRANKS );
    if (%f) {
        $mods .= "&nbsp;" if ($mods);
        $mods .= join("&nbsp;", sort { $ranks{$a} <=> $ranks{$b} } keys(%f));
        $mods .= "&nbsp;functions";
    }
    my %m = %{ $inherit->[2] };
    %ranks = ( $preserve_order ? %m : %MODRANKS );
    if (%m) {
        $mods .= "&nbsp;" if ($mods);
        $mods .= join("&nbsp;", sort { $ranks{$a} <=> $ranks{$b} } keys(%m));
    }
    return $mods;
}

sub variable_mods($) {
    my ($var, $preserve_order) = @_;
    my $mods = "";

    my %v = %{ $var->[0] };
    my %ranks = ( $preserve_order ? %v : %MODRANKS );
    if (%v) {
        $mods .= join("&nbsp;", sort { $ranks{$a} <=> $ranks{$b} } keys(%v));
    }
    return $mods;
}

sub function_mods($) {
    my ($func) = @_;
    my %f = %{ $func->[0] };
    my %ranks = ( $preserve_order ? %f : %MODRANKS );
    if (%f) {
        $mods .= join("&nbsp;", sort { $ranks{$a} <=> $ranks{$b} } keys(%f));
    }
    return $mods;
}

sub function_args($) {
    my ($func) = @_;
    my $args = "";
    my $first = 1;
    for (@{$func->[2]}) {
        $args .= ",&nbsp;" unless($first);
        $args .= "$_->[0]";
        $args .= "&nbsp;" unless ($_->[0] =~ /\*$/); 
        $args .= $_->[1];
        $first = 0;
    }
    return $args;
}

