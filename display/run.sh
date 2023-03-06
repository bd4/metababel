#!/bin/bash -x

set -e

# SINK.tally params
display_compact=true

# Generate interval messages "interval_callbacks.c"
ruby ./source_callbacks_generator.rb 3.interval_instances.yaml SOURCE.interval/interval_callbacks.c

# Generate SOURCE.invertal component
ruby ../main.rb -d 2.interval_definitions.yaml -t SOURCE -p convert --params 1.params.yaml -c interval
make -C SOURCE.interval
babeltrace2 --plugin-path=SOURCE.interval \
            --component=source.convert.interval \
            --component=sink.text.details 

# Genarate SINK.tally component
ruby ../main.rb -u 2.interval_definitions.yaml -t SINK -p display --params 1.params.yaml -c tally
make -C SINK.tally
babeltrace2 --plugin-path=SOURCE.interval:SINK.tally  \
            --component=source.convert.interval \
            --component=sink.display.tally  \
            --params="display_compact=${display_compact}"  

rm -f component.h dispatch.{h,c,o} params.{h,c,o} tally.{c,o,so} my_callbacks.o my_demangle.o

# Cleaning up
make -C SOURCE.interval clean
make -C SINK.tally clean 
