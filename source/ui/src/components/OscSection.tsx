import { Section } from './Section.js'
import { Knob } from './Knob.js'

function fmtPercent(v: number) { return v.toFixed(0) + '%' }
function fmtSemitones(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(0) + 'st' }
function fmtCents(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(1) + 'c' }

export function OscSection() {
  return (
    <Section label="OSC">
      <Knob parameterId="oscMix" label="Mix" formatValue={fmtPercent} />
      <Knob parameterId="oscTune" label="Tune" formatValue={fmtSemitones} />
      <Knob parameterId="oscFine" label="Fine" formatValue={fmtCents} />
    </Section>
  )
}
