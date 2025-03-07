[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = ../heat_source/stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  source_strength = 1e12
  solid_blocks = '100'
  fluid_blocks = '200'
  tally_blocks = '100 200'
  solid_cell_level = 0
  fluid_cell_level = 0
  tally_type = cell
  initial_properties = xml

  tally_score = flux
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [vol_solid]
    type = VolumePostprocessor
    block = '100'
  []
  [vol_fluid]
    type = VolumePostprocessor
    block = '200'
  []
  [flux_pebble1]
    type = PointValue
    variable = flux
    point = '0 0 0'
  []
  [flux_pebble2]
    type = PointValue
    variable = flux
    point = '0 0 4'
  []
  [flux_pebble3]
    type = PointValue
    variable = flux
    point = '0 0 8'
  []
  [flux_fluid]
    type = PointValue
    variable = flux
    point = '0 0 2'
  []
[]

[Outputs]
  csv = true
[]
