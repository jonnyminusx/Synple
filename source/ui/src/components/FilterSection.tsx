import { Section } from './Section.js'
import { Knob } from './Knob.js'

function fmtPercent(v: number) { return v.toFixed(0) + '%' }
function fmtSigned(v: number) { return (v >= 0 ? '+' : '') + v.toFixed(0) + '%' }
function fmtVelocity(v: number) { return v < -90 ? 'OFF' : fmtSigned(v) }

export function FilterSection() {
  return (
    <Section label="Filter" borderClass="border-indigo-400" bgClass="bg-indigo-200">
      <Knob parameterId="filterFreq" label="Freq" formatValue={fmtPercent} />
      <Knob parameterId="filterReso" label="Reso" formatValue={fmtPercent} />
      <Knob parameterId="filterEnv" label="Env" formatValue={fmtSigned} />
      <Knob parameterId="filterLFO" label="LFO" formatValue={fmtPercent} />
      <Knob parameterId="filterVelocity" label="Vel" formatValue={fmtVelocity} />
    </Section>
  )
}
