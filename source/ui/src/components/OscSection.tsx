import { Section } from './Section.js'
import { Knob } from './Knob.js'
import { Select } from './Select.js'

function fmtPercent(v: number) { return v.toFixed(0) + '%' }
function fmtSemitones(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(0) + 'st' }
function fmtCents(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(1) + 'c' }

export function Osc1Section() {
  return (
    <Section label="OSC 1" borderClass="border-red-400" bgClass="bg-red-200">
      <Select parameterId="osc1Waveform" label="Wave" />
      <Knob parameterId="osc1PulseWidth" label="PW" formatValue={fmtPercent} />
      <Knob parameterId="osc1Volume" label="Vol" formatValue={fmtPercent} />
      <Knob parameterId="osc1Tune" label="Tune" formatValue={fmtSemitones} />
      <Knob parameterId="osc1Fine" label="Fine" formatValue={fmtCents} />
    </Section>
  )
}
