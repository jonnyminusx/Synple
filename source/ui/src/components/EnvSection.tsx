import { Section } from './Section.js'
import { Knob } from './Knob.js'

interface EnvSectionProps {
  prefix: 'env' | 'filter'
  label: string
}

function fmtPercent(v: number) { return v.toFixed(0) + '%' }

export function EnvSection({ prefix, label }: EnvSectionProps) {
  return (
    <Section label={label}>
      <Knob parameterId={`${prefix}Attack`} label="A" formatValue={fmtPercent} />
      <Knob parameterId={`${prefix}Decay`} label="D" formatValue={fmtPercent} />
      <Knob parameterId={`${prefix}Sustain`} label="S" formatValue={fmtPercent} />
      <Knob parameterId={`${prefix}Release`} label="R" formatValue={fmtPercent} />
    </Section>
  )
}
