#!/usr/bin/perl
#
# usage ./ISO-6093-NR3.pl (relaxed|stric) <float>
#
# parse ISO 6093 floating point decimal representation NR3
#
# 'relaxed'
#
#   - prohibits missing decimal point. e.g. '1'              ERROR
#   - permits leading zeros on integer. e.g. '01.23e10'      OK
#   - permits trailing zeros on fraction. e.g. '1.230e10'    OK
#   - permits leading zeros on exponent. e.g. '1.23e010'     OK
#   - permits solitary point e.g. '.'                        OK
#   - permits point without succeeding fraction. e.g. '1.'   OK
#   - permits point without preceding integer. e.g. '.23'    OK
#
# 'strict'
#
#   - prohibits missing decimal point. e.g. '1'              ERROR
#   - prohibits leading zeros on integer. e.g. '01.23e10'    ERROR
#   - prohibits trailing zeros on fraction. e.g. '1.230e10'  ERROR
#   - prohibits leading zeros on exponent. e.g. '1.23e010'   ERROR
#   - prohibits solitary point e.g. '.'                      ERROR
#   - prohibits point without succeeding fraction. e.g. '1.' ERROR
#   - prohibits point without preceding integer. e.g. '.23'  ERROR
#
# C specifies that a floating-point number always contains
# a decimal-point character, but it allows leading zeros on
# the integer significand, leading zeros on the exponent,
# and trailing zeros on the fraction, similarly to 'relaxed'.
#
# JSON in rfc7159 (*1) has the constraint that the integer
# significand may not start with zero unless it is a lone
# zero, but otherwise allows leading zeros on the exponent
# and trailing zeros on the fraction, a subset of 'strict'.
#
# [1] https://tools.ietf.org/html/rfc7159.html#section-6

if ($ARGV[0] eq 'relaxed') {
    if (my ($int, $frac, undef, $exp) =
            ($ARGV[1] =~ m{
                ^
                   ( [0-9]* )                          # integer
                \. ( [0-9]* )                          # fraction
                ([eE] ( [-+]? [0-9]+ ) )?              # exponent
                $
            }x
        )) {
        printf ("{ integer:$int fraction:$frac exponent:$exp }\n");
    }
}
elsif ($ARGV[0] eq 'strict') {
    if (my ($int, undef, undef, $frac, undef, $exp) =
            ($ARGV[1] =~ m{
                ^
                   ( ([1-9][0-9]*) | 0 )               # integer
                \. ( ([0-9]*[1-9]) | 0 )               # fraction
                ([eE] ( -? ( ([1-9][0-9]*) | 0 ) ) )?  # exponent
                $
            }x
        )) {
        printf ("{ integer:$int fraction:$frac exponent:$exp }\n");
    }
}
else {
    die "usage. ./ISO-6093-NR3.pl (relaxed|stric) <float>\n";
}
