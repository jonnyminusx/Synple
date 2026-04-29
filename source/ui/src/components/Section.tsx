import type { ReactNode } from 'react'

interface SectionProps {
  label: string
  children: ReactNode
  className?: string
}

export function Section({ label, children, className = '' }: SectionProps) {
  return (
    <div className={`flex flex-col border border-zinc-700 rounded bg-zinc-800/50 ${className}`}>
      <span className="text-[9px] font-mono uppercase tracking-widest text-zinc-500 px-2 pt-1 leading-none">
        {label}
      </span>
      <div className="flex flex-wrap gap-3 p-2 pt-1.5">{children}</div>
    </div>
  )
}
