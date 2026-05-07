import type { ReactNode } from 'react'

interface SectionProps {
  label: string
  children: ReactNode
  className?: string
  borderClass?: string
  bgClass?: string
}

export function Section({ label, children, className = '', borderClass = 'border-zinc-500', bgClass = 'bg-zinc-700/40' }: SectionProps) {
  return (
    <div className={`flex flex-col border-[3px] ${borderClass} rounded ${bgClass} ${className}`}>
      <span className="text-[9px] font-mono uppercase tracking-widest text-zinc-800 px-2 pt-1 leading-none">
        {label}
      </span>
      <div className="flex items-center gap-3 p-2 pt-1.5">{children}</div>
    </div>
  )
}
