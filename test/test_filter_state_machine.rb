require 'base_test'

class TestFilterStateMachine < Test::Unit::TestCase
  include GenericTest
  extend VariableAccessor
  include VariableClassAccessor

  def self.startup
    @btx_components = [
      {
        btx_component_label: 'A',
        btx_component_type: 'SOURCE',
        btx_component_name: 'component_0',
        btx_component_plugin_name: 'plugin_0',
        btx_component_path: './test/SOURCE.plugin_0.component_0',
        btx_component_downtream_model: './test/cases_filter_state_machine/1.A.btx_model.yaml',
        btx_log_path: './test/cases_filter_state_machine/1.A.btx_log.txt'
      },
      {
        btx_component_label: 'B',
        btx_component_type: 'SOURCE',
        btx_component_name: 'component_1',
        btx_component_plugin_name: 'plugin_1',
        btx_component_path: './test/SOURCE.plugin_1.component_1',
        btx_component_downtream_model: './test/cases_filter_state_machine/1.B.btx_model.yaml',
        btx_log_path: './test/cases_filter_state_machine/1.B.btx_log.txt'
      },
      {
        btx_component_label: 'C',
        btx_component_type: 'FILTER',
        btx_component_name: 'filter_component',
        btx_component_plugin_name: 'filter_plugin',
        btx_component_upstream_model: [
            './test/cases_filter_state_machine/1.A.btx_model.yaml',
            './test/cases_filter_state_machine/1.B.btx_model.yaml'
        ],
        btx_component_downtream_model: './test/cases_filter_state_machine/1.btx_model.yaml',
        btx_file_usr_callbacks: './test/cases_filter_state_machine/1.callbacks.c'
      },
      {
        btx_component_label: 'D',
        btx_component_type: 'SINK',
        btx_component_plugin_name: 'utils',
        btx_component_name: 'dummy',
        # Prevent the component compilation which is not needed for babeltrace components.
        btx_compile: false
      }
    ]

    @btx_connect = [ 'A:C', 'B:C', 'C:D' ]
  end
end