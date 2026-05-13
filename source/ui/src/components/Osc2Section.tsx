import { Section } from './Section.js'
import { Knob } from './Knob.js'
import { Select } from './Select.js'

function fmtPercent(v: number) { return v.toFixed(0) + '%' }
function fmtSemitones(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(0) + 'st' }
function fmtCents(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(1) + 'c' }

export function Osc2Section() {
  return (
    <Section label="OSC 2" borderClass="border-orange-400" bgClass="bg-orange-200">
      <Select parameterId="osc2Waveform" label="Wave" />
      <Knob parameterId="osc2PulseWidth" label="PW" formatValue={fmtPercent} />
      <Knob parameterId="osc2Volume" label="Vol" formatValue={fmtPercent} />
      <Knob parameterId="osc2Tune" label="Tune" formatValue={fmtSemitones} />
      <Knob parameterId="osc2Fine" label="Fine" formatValue={fmtCents} />
    </Section>
  )
}
