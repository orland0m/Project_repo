/*Orlando*/

/* URI Grammar

URI = [ absoluteURI | relativeURI ] [ "#" fragment ]

       absoluteURI = scheme ":" ( hierarchical_part | opaque_part )

       relativeURI = ( net_path | absolute_path | relative_path ) [ "?" query ]

       scheme = "http" | "ftp" | "gopher" | "mailto" | "news" | "telnet" |
                  "file" | "man" | "info" | "whatis" | "ldap" | "wais" | ...

       hierarchical_part = ( net_path | absolute_path ) [ "?" query ]

       net_path = "//" authority [ absolute_path ]

       absolute_path = "/"  path_segments

       relative_path = relative_segment [ absolute_path ] */
