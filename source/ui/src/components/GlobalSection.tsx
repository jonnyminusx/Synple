import { Section } from './Section.js'
import { Knob } from './Knob.js'
import { Select } from './Select.js'

function fmtOctave(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(0) }
function fmtCents(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(1) + 'c' }
function fmtDb(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(1) + 'dB' }
function fmtPercent(v: number) { return v.toFixed(0) + '%' }

export function GlobalSection() {
  return (
    <Section label="Global" borderClass="border-cyan-400" bgClass="bg-cyan-200">
      <Knob parameterId="noise" label="Noise" formatValue={fmtPercent} />
      <Knob parameterId="octave" label="Oct" formatValue={fmtOctave} />
      <Knob parameterId="tuning" label="Tune" formatValue={fmtCents} />
      <Knob parameterId="outputLevel" label="Level" formatValue={fmtDb} />
      <Select parameterId="polyMode" label="Mode" />
    </Section>
  )
}
