# Dynamic Structured Data (DSD)

Dynamic Structured Data (DSD) is an meta-transfer syntax intended to
facilitate the transfer of information between loosely coupled
systems. DSD defines three concrete transfer syntaxes (Text, Binary
and XML), an Abstract Type System and three concrete type systems. The
DSD Abstract Type System defines type semantics for five atomic types
and two collection types.

DSD is intended to decouple type semantics from transfer syntax. It
is, by itself type-system neutral (DSD does not define how many bits
encode an integer or how floating point values are laid out in
memory.) However, DSD can carry an annotation that the creator of a
message assumes type details useful for 8-bit sensors, 32-bit
micro-controllers and 64-bit server systems.

This package provides a Lexxer and example parser for the Text and
Binary transfer syntaxes. (Users of the XML transfer syntax can simply
use their favourite XML parser.)

DSD is defined independently of this package, but a copy of it's
official definition is included in the file
[dsd-info.txt](dsd-info.txt). For a more literate introduction to the
problem(s) DSD was trying to solve, see [Goldilocks and the 11
Parsers](goldilocks.txt). DSD is based on the Virtual Worlds Region
Agent Protocol Abstract (VWRAP) Type System defined in
https://tools.ietf.org/html/draft-ietf-vwrap-type-system-00. For the
truly masochistic, details of the VWRAP effort are available at
http://meadhbh.hamrick.rocks/home/technical-sh-t/what-is-vwrap.

## DSD In 120 Seconds

DSD defines a syntax for protocol messages between systems. In this
sense it's sort of like JSON or XML (though there is a Binary transfer
syntax that's a little more efficient to parse.) Here's an example of a
typical DSD message:

    # Login Authentication w/ No Iteration Count or Salt
    @t
    {
      "username" = "OhMeadhbh"
      "secret" = (
        b6:07:54:c4:ea:1a:fa:24:20:1e
        af:02:4c:13:0d:94:cc:58:01:65
      )
      "algorithm" = "sha1"
      "version" = 2
    }

From this example you can probably deduce a couple things:

* The Hash mark starts a comment

* Text strings are delimited by double quotes (")

* Maps are bounded by the '{' and '}' characters

* Map Keys and Map Values are linked with the equals character (=)

* You don't need to use commas (because DSD/Text is "self-delimiting")

* Annotations begin with the at symbol (@)

If you walk away from this document remembering "DSD Text is sort of
like JSON with comments and no commas" and "The OhMeadhbh/DSD package
provides a lexxer for the DSD Text transfer syntax" then you've
learned enough for practical use of DSD as a syntax and this
software.

Since you're still reading this section, it probably means you're
interested in more details. Here's an interesting one: the message
above can be encoded using DSD/Binary into these 78 hexadecimal
octets:

    02 68 42 47 75 73 65 72 6e 61 6d 65 48 00 4f 68
    4d 65 61 64 68 62 68 45 73 65 63 72 65 74 48 0B
    b6 07 54 c4 ea 1a fa 24 20 1e af 02 4c 13 0d 94
    cc 58 01 65 48 00 61 6c 67 6f 72 69 74 68 6d 43
    73 68 61 31 46 76 65 72 73 69 6f 6e 10 02

It's interesting to note that the Binary encoding seldom makes
messages much shorter, but it often makes them easier to parse.

If you're in an environment which uses XML extensively, you can use
the XML transfer syntax:

    <!-- Login Authentication w/ No Iteration Count or Salt
    <type>tiny</type>
    <map>
      <key>username</key>
      <string>OhMeadhbh</string>

      <key>secret</key>
      <base64>MDEyMzQ1Njc4OTAxMjM0NTY3ODk=</base64>

      <key>algorithm</key>
      <string>sha1</string>

      <key>version</key>
      <integer>2</integer>
    </map>

If you want to see a beefier example of a DSD file (that includes
comments, see [example.dsd](example.dsd).

If you're wondering why the world needed another format that looks
like JSON, the answer is obviously "all the other ones are wrong."
Really what happened is at Linden Lab there were significant
disagreements over whether protocol should be carried over ProtoBufs,
JSON, XML or a custom Binary format. The one abstract type system /
multiple concrete transfer syntaxes separation was created so our
distributed application could be transfer-syntax agnostic. It doesn't
matter which transfer syntax you use (Text, Binary or XML), they can
all be de-serialized into byte identical data structures.

The DSD Text format itself is slightly easier to parse on 8-bit
micro-controllers used in related projects and it natively supports
comments.



## DSD/Text Lexxer Theory of Operation
