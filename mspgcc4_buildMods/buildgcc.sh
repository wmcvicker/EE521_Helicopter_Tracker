#!/bin/sh

# This work has been partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

echo "---------------------------------------------"
echo "Please use buildgcc.pl instead of buildgcc.sh"
echo "Running buildgcc.pl..."
echo "---------------------------------------------"

exec perl "$(dirname "$0")/buildgcc.pl" "$@"
