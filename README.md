# entity-block

Converts a VHDL entity to a nice looking image in .svg format.

# build

Dependencies: Qt5. On a fresh Ubuntu install you can install the dependencies like this:

    sudo apt install build-essential qt5-default cmake

There are two possible ways to build entity-block:

1. With qmake:


    qmake .
    make
    #If you want to install system wide:
    sudo make install 

2. With cmake


    cmake .
    make
    #If you want to install system wide:
    sudo make install


# Usage

    Usage: ./entity-block [options] input output
    Reads a vhdl file and outputs a .svg file with the entity block
    (All command line options will be stored)
    
    Options:
      -h, --help                        Displays this help.
      -v, --version                     Displays version information.
      -c, --comment-color <color>       Change default comment color to <color>
      -n, --port-name-color <color>     Change default port name color to <color>
      -t, --port-type-color <color>     Change default port type color to <color>
      -b, --background-color <color>    Change default background color to <color>
      -l, --header-left-color <color>   Change default left (gradient) in the
                                        header color to <color>
      -r, --header-right-color <color>  Change default right (gradient) in the
                                        header color to <color>
      -e, --entity-title-color <color>  Change default entity color to <color>
      -B, --border-color <color>        Change default border color to <color>
      -p, --port-color <color>          Change default port symbol color to <color>
      -R, --corner-radius <number>      Change default corner radius to <number>
      -s, --shadow-color <color>        Change default shadow color to <color>
      -w, --line-weight <number>        Change default line thickness to <number>
    
    Arguments:
      input                             VHDL file to convert
      output                            SVG file to output
    
The shadow (or any other object) can be removed completely by setting the alpha value to 0
    ./entity-block CrcGenerator.vhd -s "#00FFFFFF"

# Example

This entity:

    entity CrcGenerator is
      generic (
        REVERSEINPUT  : std_logic                     := '1'; --Reverse 8 bits in input byte
        REVERSEOUTPUT : std_logic                     := '1'; --Reverse 32 bits in CRC output
        INITVAL       : std_logic_vector(31 downto 0) := X"FFFFFFFF"; --Initial value of the CRC when sReset 
        FINALXOR      : std_logic_vector(31 downto 0) := X"FFFFFFFF"; --Set to FFFFFFFF to invert CRC output
        -- Generator polynomial is
        POLYNOMIAL    : std_logic_vector(31 downto 0) := X"04C11DB7"
        );
      port (
        clk     : in  std_logic; -- 125 MHz ethernet clock
        reset_n : in  std_logic; -- asynchronous reset (active low)
        sReset  : in  std_logic; -- synchronous reset to restart CRC
        en      : in  std_logic; -- Include byte in 
        din     : in  std_logic_vector (7 downto 0); --Input byte
        dv      : out std_logic; -- CRC valid indicator
        dout    : out std_logic_vector (31 downto 0)  -- CRC output
        );
    end CrcGenerator;

converts into this:

![CrcGenerator.svg](./CrcGenerator.svg)

## Special placement of ports
* Input ports will be placed on the left side
* Output ports will be placed on the right side
* Input ports with the keyword rst or reset in their name will be grouped together
* Input ports with the keyword clk or clock in their name will be grouped together
* Ports with the keywords s_axi or slave, as well as ports with "L " in their comment will be placed on the left side
* Ports with the keywords m_axi or master, as well as ports with "R " in their comment will be placed on the right side

# Known issues

* The application does not work without a graphical session (X-server etc).
    * To work around this issue, start entity block with the argument `-platform offscreen`
* The paint function is called twice for now, in order to determine the SVG size, then it is drawn again. 
    * This could be done a little neater but it works.
    
# License

GPL v3 [LICENSE](./LICENSE).
