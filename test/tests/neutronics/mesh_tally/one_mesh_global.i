[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  solid_blocks = '100'
  initial_properties = xml
  verbose = true
  solid_cell_level = 0
  normalize_by_global_tally = true

  tally_type = mesh
  mesh_template = '../meshes/sphere.e'
  power = 100.0
  check_tally_sum = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp'
[]
