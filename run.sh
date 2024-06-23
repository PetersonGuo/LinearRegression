unset -v testdata
unset -v output
unset -v filename
unset -v extension

while getopts o:t: opt; do
	case $opt in
		o) output=$OPTARG ;;
		t) testdata=$OPTARG ;;
		*)
			echo 'Error in command line parsing' >&2
			exit 1
	esac
done

shift "$(( OPTIND - 1 ))"

if [ -z "$1" ]; then
	echo 'Missing input file' >&2
	exit 1
elif [ ! -f "$1" ]; then
	echo "Input file '$1' does not exist" >&2
	exit 1
elif [ -z "$testdata" ]; then
	testdata="testdata.txt"
fi

if [ -z $output ]; then
	output="out.txt"
fi

if [ ! -f "$testdata" ]; then
	echo "Test data file '$testdata' does not exist" >&2
	exit 1
fi

filename=$(basename -- "$1")
extension="${filename##*.}"
filename="${filename%.*}"

if g++ $1 -std=c++20 -I/opt/anaconda3/bin/python -lpython -I/home/petersonguo/.local/lib/python3.12/site-packages/numpy/core/include -shared -framework Python -g -o $filename;
then
	./$filename $testdata $output
fi
