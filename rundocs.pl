#!/usr/bin/perl

# FUTURE
# add support for: @see, @since, @link, @deprecated, @inheritDoc
# refactor, comment, etc
# re-style the thing to match the blog

# TODO
# package docs

use File::Find;
use File::Path;
use File::Basename;
use Getopt::Std;
use Data::Dumper;

%opts = ();
getopts('p', \%opts);

$JCPP = "jcpp";
$CYGROOT = "/cygdrive/c/Users/bobal_000/work/gabbo/mudlib";
$ROOT= `cygpath -w $CYGROOT`;
chomp $ROOT;
$ROOT =~ s/\\/\\\\/g;
$DOCS = "/cygdrive/c/Users/bobal_000/work/gabbo-docs/docs/mudlib";
$TMPFILE = "/tmp/lpcdoc";

@SOURCE = ( "$CYGROOT/lib", "$CYGROOT/modules", "$CYGROOT/obj", "$CYGROOT/secure" );
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

%programs = ();
%packages = ();

find(\&process_file, @SOURCE);

for (keys(%packages)) {
    &generate_package_frame($_);
    &generate_package_summary($_);
}

&generate_package_frame();
&generate_overview();
&generate_summary();

exit 0;

sub process_file {
    return unless (/\.c$/);
    my $program = $File::Find::name;
    $program =~ s/^$CYGROOT(.*)\.c$/$1/;
    print "found $program\n";
    if ($program =~ m#(.*)/package-info$#) {
      &read_package_info($1);
    } else {
      &generate_program_doc($program);
    }
}

sub read_package_info($) {
    my ($package) = @_;
    $packages{$package} = [ undef, { } ] unless (exists($packages{$package}));

    my $src = "";
    open(F, "$CYGROOT/$package/package-info.c") or die("Couldn't open $CYGROOT/$package/package-info.c for read: $!\n");
    $src .= $_ while(<F>);
    close(F);

    if ($src =~ m#/\*\*(.*?)\*/#s) {
        my $doc = &parse_doc($1);
        $packages{$package}->[0] = $doc->[0];
    }
    return;
}

# write out a new doc for the specified program
sub generate_program_doc($) {
    my ( $program ) = @_;

    $programs{$program} = undef;
    my $package = dirname($program);
    $packages{$package} = [ undef, { } ] unless (exists($packages{$package}));
    $packages{$package}->[1]->{$program} = 1;

    my $cygpath = `cygpath -w $CYGROOT$program.c`;
    $cygpath =~ s/\\/\\\\/g;

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

    $programs{$program} = [ $desc, undef, undef, undef ];

    # grab all the inherit statements before we strip out the strings
    my %inherits = ( );
    my $i = 1;
    while ($src =~ /^(.*?inherit.*?);/gm) {
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

    # now that we have all the inherits, recurse down the tree and
    # fill out the program info for the ancestors to use later for
    # cross referencing links and such
    $programs{$program}->[1] = \%inherits;
    $programs{$program}->[2] = \%variables;
    $programs{$program}->[3] = \%functions;
    my @inh = sort {
        $inherits{$a}->[3] <=> $inherits{$b}->[3]
    } keys(%inherits);
    foreach (@inh) {
        &generate_program_doc($_) unless (exists($programs{$_}));
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

# parse out documentation from comment
sub parse_doc($) {
    my ( $doc ) = @_;
    # remove the customary leading asterisk
    $doc =~ s#^\s*\*\s+##gm;
    # parse out the description
    my $desc = undef;
    if ($doc =~ /(.*?)\s*(\@.*)/ms) {
      $desc = $1;
      $doc = $2;
    } else {
        $desc = $doc;
        $doc = "";
    }
    my %tags = ();
    my $tag = undef;
    my $arg = undef;
    my $buf = "";
    # go line-by-line looking for @tags
    foreach (split /\n/, $doc) {
        if (/^\@(\w+)\s+(.*)/) {
            # found a new tag, store what we've parsed so far and reset
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
                # @param tag takes an arg
                $arg =~ /^(\w+)\s+(.*)/;
                $arg = $1;
                $buf .= "$2 ";
            } else {
                $buf .= "$arg ";
                $arg = undef;
            }
        } else {
            # no tags, append line to buffer
            s/\s*$/ /;
            $buf .= "$_";
        }
    }

    # our loop broke before the last tag was stored, do it now
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

# actually compose all the html
sub write_doc($$$) {
    my ($program, $desc, $inherits, $functions, $variables) = @_;
    my $out = "";

    # for building relative paths
    my $rel = substr("/.." x (scalar(split(/\/+/, $program)) - 2), 1);

    # omit private members if no -p option
    my @visible_variables = keys(%$variables);
    if (!exists($opts{'p'})) {
        @visible_variables = grep { 
            !exists($variables->{$_}->[0]->{'private'})
        } @visible_variables;
    }
    my @visible_functions = keys(%$functions);
    if (!exists($opts{'p'})) {
        @visible_functions = grep { 
            !exists($functions->{$_}->[0]->{'private'})
        } @visible_functions;
    }

    # start building the page
    $out .= <<END;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html lang="en">
<head>
<title>$class</title>
<link rel="stylesheet" type="text/css" href="$rel/stylesheet.css" title="Style">
</head>
<body>
END

    $out .= &navbar("top", $program);
    $out .= <<END;
<!-- ======== START OF CLASS DATA ======== -->
<div class="header">
END

    my $alias = $programs{$program}->[0]->[1]->{'alias'}->[0]->[1];
    if ($alias) {
        $out .= <<END;
<div class="subTitle">$program</div>
<h2 title="Program $alias" class="title">Program $alias</h2>
END
    } else {
        $out .= <<END;
<h2 title="Program $program" class="title">Program $program</h2>
END
    }

    $out .= <<END;
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
END
        $out .= &inherit_tree($inherits, $rel);
        $out .= <<END;
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
    my $inherited_variables = "";
    if (%$inherits) {
        $inherited_variables .= &inherited_variables($inherits, $rel, {});
    }

    if (@visible_variables || $inherited_variables) {
        $out .= <<END;
<!-- =========== FIELD SUMMARY =========== -->
<ul class="blockList">
<li class="blockList"><a name="field_summary">
<!--   -->
</a>
<h3>Variable Summary</h3>
END
        if (@visible_variables) {
            $out .= <<END;
<table class="overviewSummary" border="0" cellpadding="3" cellspacing="0" summary="Variable Summary table, listing fields, and an explanation">
<caption><span>Variables</span><span class="tabEnd">&nbsp;</span></caption>
<tr>
<th class="colFirst" scope="col">Modifier and Type</th>
<th class="colLast" scope="col">Variable and Description</th>
</tr>
END
            my $color;
            my @vars = sort @visible_variables;
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
END
        }
        $out .= <<END;
$inherited_variables
</li>
</ul>
END
    }

    my $inherited_functions = "";
    if (%$inherits) {
        $inherited_functions .= &inherited_functions($inherits, $rel, {});
    }
    if (@visible_functions || $inherited_functions) {
        $out .= <<END;
<!-- ========== METHOD SUMMARY =========== -->
<ul class="blockList">
<li class="blockList"><a name="method_summary">
<!--   -->
</a>
<h3>Function Summary</h3>
END
        if (@visible_functions) {
            $out .= <<END;
<table class="overviewSummary" border="0" cellpadding="3" cellspacing="0" summary="Function Summary table, listing functions, and an explanation">
<caption><span>Functions</span><span class="tabEnd">&nbsp;</span></caption>
<tr>
<th class="colFirst" scope="col">Modifier and Type</th>
<th class="colLast" scope="col">Function and Description</th>
</tr>
END
            my @funcs = sort @visible_functions;
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
END
        }
        $out .= <<END;
$inherited_functions
</li>
</ul>
END
    }
    $out .= <<END;
</li>
</ul>
</div>
<div class="details">
<ul class="blockList">
<li class="blockList">
END

    if (@visible_variables) {
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
        } @visible_variables;
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

    if (@visible_functions) {
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
        } @visible_functions;
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

    $out .= navbar("bottom", $program);

    $out .= <<END;
</body>
</html>
END

    return $out;
}

sub navbar($$) {
    my ($loc, $program, $package) = @_;
    my $rel = substr("/.." x (scalar(split(/\/+/, $program)) - 1), 1);
    $rel = "." unless ($rel);
    my $out = "";
    $out .= <<END;
<div class="${loc}Nav"><a name="navbar_$loc">
</a><a href="#skip-navbar_$loc" title="Skip navigation links"></a><a name="navbar_$loc_firstrow">
</a>
<ul class="navList" title="Navigation">
<!--
<div class="aboutLanguage"><em><strong>gabbo mudlib foundation<br/>version 0.1</strong></em></div>
-->
</ul>
</div>
<div class="subNav">
END
    if ($package) {
        $out .= <<END;
<!--
<ul class="navList">
<li><a href="#" title="class in java.lang"><span class="strong">Prev Program</span></a></li>
<li><a href="#" title="class in java.lang"><span class="strong">Next Program</span></a></li>
</ul>
-->
END
    } else {
        $out .= <<END;
<ul class="navList">
<li>Prev</li>
<li>Next Program</li>
</ul>
END
    }
    $out .= <<END;
<ul class="navList">
<li><a href="$rel/index.html?$program.html" target="_top">Frames</a></li>
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
END
    unless ($package) {
        $out .= <<END;
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
END
    }
    $out .= <<END;
<a name="skip-navbar_$loc">
<!--   -->
</a></div>
END
    return $out;
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

sub inherit_tree($) {
    my ($inherits, $rel) = @_;
    my $out = "<ul class=\"tree\">";

    my @inh = sort {
        $inherits->{$a}->[3] <=> $inherits->{$b}->[3]
    } keys(%$inherits);
    for (my $i = 0; $i <= $#inh; $i++) {
        my $last = " class=\"last\"" if ($i == $#inh);
        my $prog = $inh[$i];
        my $mods = &inherit_mods($inherits->{$prog});
        $mods = "&nbsp;[$mods]" if ($mods);
        if ($programs{$prog}->[0]) {
            $out .= <<END;
<li$last><a href="$rel$prog.html" title="">$programs{$prog}->[0]</a>$mods
END
        } else {
            $out .= <<END;
<li$last><a href="$rel$prog.html" title="">$prog</a>$mods
END
        }

        if (%{$programs{$prog}->[1]}) {
            # recurse down the inheritance tree
            $out .= &inherit_tree($programs{$prog}->[1], $rel);
        }
        $out .= "</li>";
    }
    $out .= "</ul>";
    return $out;
}

sub inherited_variables($) {
    my ($inherits, $rel, $found) = @_;
    my @inh = sort {
        $inherits->{$a}->[3] <=> $inherits->{$b}->[3]
    } keys(%$inherits);
    my $out = "";
    for $program (@inh) {
        next if (exists($found->{$program}));
        next if (exists($inherits->{$program}->[0]->{'private'}));
        my $buf = "";
        my @vars = sort keys(%{$programs{$program}->[2]});
        my $first = 1;
        foreach (@vars) {
            my $var = $programs{$program}->[2]->{$_};
            next if (exists($var->[0]->{'private'}));
            my $comma = ", " unless($first);
            $buf .= "$comma<a href=\"$rel$program.html#$_\">$_</a>";
            $first = 0;
        }
        if ($buf) {
            my $alias = $programs{$program}->[0]->[1]->{'alias'}->[0]->[1];
            $alias = $program unless($alias);
            $buf = <<END;

<li class="blockList"><a name="fields_inherited_from_class_$program">
</a>
<h3>Variables inherited from program&nbsp;<a href="$rel$program.html" title="program $program">$alias</a></h3>
<code>$buf</code></li>
END
        }
        if ($buf) {
            $buf = <<END;
<ul class="blockList">
$buf
</ul>
END
        }
        $out .= $buf;
        $found->{$program} = 1;
        $out .= &inherited_variables($programs{$program}->[1], $rel, $found);
    }
    return $out;
}

sub inherited_functions($) {
    my ($inherits, $rel, $found) = @_;
    my @inh = sort {
        $inherits->{$a}->[3] <=> $inherits->{$b}->[3]
    } keys(%$inherits);
    my $out = "";
    for $program (@inh) {
        next if (exists($found->{$program}));
        next if (exists($inherits->{$program}->[1]->{'private'}));
        my $buf = "";
        my @funcs = sort keys(%{$programs{$program}->[3]});
        my $first = 1;
        foreach (@funcs) {
            my $func = $programs{$program}->[3]->{$_};
            next if (exists($func->[0]->{'private'}));
            my $comma = ", " unless($first);
            $buf .= "$comma<a href=\"$rel$program.html#$_()\">$_</a>";
            $first = 0;
        }
        if ($buf) {
            my $alias = $programs{$program}->[0]->[1]->{'alias'}->[0]->[1];
            $alias = $program unless($alias);
            $buf = <<END;
<li class="blockList"><a name="methods_inherited_from_class_$program">
</a>
<h3>Functions inherited from program&nbsp;<a href="$rel$program.html" title="program $program">$alias</a></h3>
<code>$buf</code></li>
END
        }
        if ($buf) {
            $buf = <<END;
<ul class="blockList">
$buf
</ul>
END
        }
        $out .= $buf;
        $found->{$program} = 1;
        $out .= &inherited_functions($programs{$program}->[1], $rel, $found);
    }
    return $out;
}

sub generate_overview() {
    my $out = <<END;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html lang="en">
<head>
<title>Overview List (gabbo mudlib foundation)</title>
<link rel="stylesheet" type="text/css" href="stylesheet.css" title="Style">
</head>
<body>
<h1 title="gabbo mudlib foundation" class="bar"><strong>gabbo mudlib foundation</strong></h1>
<div class="indexHeader"><a href="allclasses-frame.html" target="packageFrame">All Programs</a></div>
<div class="indexContainer">
<h2 title="Packages">Packages</h2>
<ul title="Packages">
END

    my @packages = sort keys(%packages);
    foreach (@packages) {
        $out .= <<END;
<li><a href=".$_/package-frame.html" target="packageFrame">$_</a></li>
END
    }

    $out .= <<END;
</ul>
</div>
<p>&nbsp;</p>
</body>
</html>
END

    open (F, ">$DOCS/overview-frame.html") or die("Couldn't open overview-frame.html for write: $!\n");
    print F $out;
    close(F);
}

sub generate_package_frame($) {
    my ($package) = @_;
    my $rel = substr("/.." x (scalar(split(/\/+/, $package)) - 1), 1);
    $rel = "." unless ($rel);
 
    my $out = "";
    $out .= <<END;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html lang="en">
<head>
<title>
END
    $out .= ($package ? $package : "All Programs");
    $out .= <<END;
(gabbo mudlib foundation)</title>
<link rel="stylesheet" type="text/css" href="$rel/stylesheet.css" title="Style">
</head>
<body>
END
    if ($package) {
        $out .= <<END;
<h1 class="bar"><a href="$rel$package/package-summary.html" target="classFrame">$package</a></h1>
<div class="indexContainer">
<ul>
END
    } else {
        $out .= <<END;
<h1 class="bar">All Programs</h1>
<div class="indexContainer">
<ul>
END
    }
    my @programs = undef;
    if ($package) {
        @programs = keys(%{$packages{$package}->[1]});
    } else {
        @programs = keys(%programs);
    }
    @programs = sort {
        ( $programs{$a}->[0] ? $programs{$a}->[0] : basename($a) ) 
        cmp
        ( $programs{$b}->[0] ? $programs{$b}->[0] : basename($b) ) 
    } @programs;
    foreach my $program (@programs) {
        my $alias = $programs{$program}->[0]->[1]->{'alias'}->[0]->[1];
        $alias = basename($program) unless($alias);
        $out .= <<END;
<li><a href="$rel/$program.html" title="program in $package" target="classFrame">$alias</a></li>
END
    }
    $out .= <<END;
</ul>
</div>
</body>
</html>
END

    my $file = "$DOCS/allclasses-frame.html";
    $file = "$DOCS$package/package-frame.html" if ($package);
    mkpath(dirname($file));
    open (F, ">$file") or die("Couldn't open $file for write: $!\n");
    print F $out;
    close(F);
}

sub generate_package_summary($) {
    my ($package) = @_;
    my $out = "";

    my $rel = substr("/.." x (scalar(split(/\/+/, $package)) - 1), 1);
    $rel = "." unless ($rel);

    my $summary = $packages{$package}->[0];
    $summary =~ s/^(.*?\.)\s.*/$1/s;

    $out .= <<END;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<!-- NewPage -->
<html lang="en">
<head>
<title>$package (gabbo mudlib foundation)</title>
<link rel="stylesheet" type="text/css" href="$rel/stylesheet.css" title="Style">
</head>
<body>
<script type="text/javascript"><!--
    if (location.href.indexOf('is-external=true') == -1) {
        parent.document.title="$package (gabbo mudlib foundation)";
    }
//-->
</script>
<noscript>
<div>JavaScript is disabled on your browser.</div>
</noscript>
<!-- ========= START OF TOP NAVBAR ======= -->
END
    $out .= &navbar($top, $package, 1);
    $out .= <<END;
<!-- ========= END OF TOP NAVBAR ========= -->
<div class="header">
<h1 title="Package" class="title">Package&nbsp;$package</h1>
<div class="docSummary">
<div class="block">$summary</div>
</div>
<p>See:&nbsp;<a href="#package_description">Description</a></p>
</div>
<div class="contentContainer">
<ul class="blockList">
<li class="blockList">
<table class="packageSummary" border="0" cellpadding="3" cellspacing="0" summary="Package Summary table, listing programs, and an explanation">
<caption><span>Program Summary</span><span class="tabEnd">&nbsp;</span></caption>
<tr>
<th class="colFirst" scope="col">Program</th>
<th class="colLast" scope="col">Description</th>
</tr>
<tbody>
END
    my @programs = undef;
    if ($package) {
        @programs = keys(%{$packages{$package}->[1]});
    } else {
        @programs = keys(%programs);
    }
    @programs = sort {
        ( $programs{$a}->[0] ? $programs{$a}->[0] : basename($a) ) 
        cmp
        ( $programs{$b}->[0] ? $programs{$b}->[0] : basename($b) ) 
    } @programs;
    my $color = "";
    foreach my $program (@programs) {
        my $alias = $programs{$program}->[0]->[1]->{'alias'}->[0]->[1];
        $alias = basename($program) unless($alias);
        if ($color eq "row") { $color = "alt"; }
        else { $color = "row"; }
        my $summary = $programs{$program}->[0]->[0];
        $summary =~ s/^(.*?\.)\s.*/$1/s;
        $out .= <<END;        
<tr class="$colorColor">
<td class="colFirst"><a href="$rel$program.html" title="program in $package">$alias</a></td>
<td class="colLast">
<div class="block">$summary</div>
</td>
</tr>
END
    }
    $out .= <<END;
</tbody>
</table>
</li>
</ul>
<a name="package_description">
<!--   -->
</a>
<h2 title="Package $package Description">Package $package Description</h2>
<div class="block">$packages{$package}->[0]</div>
<!-- ======= START OF BOTTOM NAVBAR ====== -->
END
    $out .= &navbar("bottom", $package, 1);
    $out .= <<END;
<!-- ======== END OF BOTTOM NAVBAR ======= -->
</body>
</html>
END

    my $file = "$DOCS$package/package-summary.html";
    mkpath(dirname($file));
    open (F, ">$file") or die("Couldn't open $file for write: $!\n");
    print F $out;
    close(F);
}

sub generate_summary() {
    my $out = "";
    $out .= <<END;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<!-- NewPage -->
<html lang="en">
<head>
<title>Overview (gabbo mudlib foundation)</title>
<link rel="stylesheet" type="text/css" href="stylesheet.css" title="Style">
</head>
<body>
<script type="text/javascript"><!--
    if (location.href.indexOf('is-external=true') == -1) {
        parent.document.title="Overview (gabbo mudlib foundation)";
    }
//-->
</script>
<noscript>
<div>JavaScript is disabled on your browser.</div>
</noscript>
<!-- ========= START OF TOP NAVBAR ======= -->
END
    $out .= &navbar("top", "", 1);
    $out .= <<END;
<!-- ========= END OF TOP NAVBAR ========= -->
<div class="header">
<h1 class="title">gabbo mudlib foundation<br>API Specification</h1>
</div>
<div class="header">
<div class="subTitle">
<div class="block">This document is the API specification for the gabbo mudlib foundation.</div>
</div>
<p>See: <a href="#overview_description">Description</a></p>
</div>
<div class="contentContainer">
<table class="overviewSummary" border="0" cellpadding="3" cellspacing="0" summary="Packages table, listing packages, and an explanation">
<caption><span>Packages</span><span class="tabEnd">&nbsp;</span></caption>
<tr>
<th class="colFirst" scope="col">Package</th>
<th class="colLast" scope="col">Description</th>
</tr>
<tbody>
END
    my $color = "";
    for (sort(keys(%packages))) {
        my $summary = $packages{$_}->[0];
        $summary =~ s/^(.*?\.)\s.*/$1/s;
        if ($color eq "row") { $color = "alt"; }
        else { $color = "row"; }
        $out .= <<END;
<tr class="$colorColor">
<td class="colFirst"><a href=".$_/package-summary.html">$_</a></td>
<td class="colLast">
<div class="block">$summary</div>
</td>
</tr>
END
    }
    $out .= <<END;
</tbody>
</table>
</div>
<div class="footer"><a name="overview_description">
<!--   -->
</a>
<div class="subTitle">
<div class="block">This document is the API specification for the Java&#x2122;
Platform, Standard Edition.</div>
</div>
</div>
<!-- ======= START OF BOTTOM NAVBAR ====== -->
END
    $out .= &navbar($bottom, "", 1);
    $out .= <<END;
<!-- ======== END OF BOTTOM NAVBAR ======= -->
</body>
</html>
END
    my $file = "$DOCS/overview-summary.html";
    open (F, ">$file") or die("Couldn't open $file for write: $!\n");
    print F $out;
    close(F);
}

