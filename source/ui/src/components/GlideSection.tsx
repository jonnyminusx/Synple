import { Section } from './Section.js'
import { Knob } from './Knob.js'
import { Select } from './Select.js'

function fmtPercent(v: number) { return v.toFixed(0) + '%' }
function fmtSemitones(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(0) + 'st' }

export function GlideSection() {
  return (
    <Section label="Glide">
      <Select parameterId="glideMode" label="Mode" />
      <Knob parameterId="glideRate" label="Rate" formatValue={fmtPercent} />
      <Knob parameterId="glideBend" label="Bend" formatValue={fmtSemitones} />
    </Section>
  )
}
